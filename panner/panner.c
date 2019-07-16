
#include <localdef.h>

/* $TOG: panner.c /main/6 1997/03/31 13:38:32 dbl $ */
/*
 * @OPENGROUP_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF for
 * the full copyright text.
 * 
 * This software is subject to an open license. It may only be
 * used on, with or for operating systems which are themselves open
 * source systems. You must contact The Open Group for a license
 * allowing distribution and sublicensing of this software on, with,
 * or for operating systems which are not Open Source programs.
 * 
 * See http://www.opengroup.org/openmotif/license for full
 * details of the license agreement. Any use, reproduction, or
 * distribution of the program constitutes recipient's acceptance of
 * this agreement.
 * 
 * EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 * PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 * WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 * OR FITNESS FOR A PARTICULAR PURPOSE
 * 
 * EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 * NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 * EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 */
/*
 * HISTORY
 */
#include <config.h>
#include <localdef.h>

#include <stdio.h>
#include <stdlib.h>
#ifdef ADD_PAN
#include <math.h>
#endif
#include <X11/Xmd.h>
#include <Xm/TransferP.h>
#include <Xm/AtomMgr.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/Notebook.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/XmStrDefs.h>

#ifdef ADD_PAN

#define INIT_SCREEN_WIDTH	100
#else
#define INIT_SCREEN_WIDTH	50
#endif
#define MAX_DISPLAY_COUNT	10
#define LOCAL			0
#ifdef ADD_PAN
#define PAN_ELEM		7           /* also give PanRoot x,y,w,h */
#define PAN_FORMAT		32
#else
          /* note 9 not 3, as format is 8 */
          /* damages X session if wrong ? */
#define PAN_ELEM		9
#define PAN_FORMAT		8
#define extraGC  canvasGC
#endif
#define WM_SELECTION_FORMAT	"WM_S%1d"
#define COLOR_COUNT             20


String fallback[] = {
  "Panner.mappedWhenManaged:		FALSE",

  "Panner.width:			325",
  "Panner.height:			290",
  "Panner*frame.marginWidth:		7",
  "Panner*frame.marginHeight:		7",
  "Panner*notebook.firstPageNumber:	0",
  "Panner*tab.shadowThickness:		1",
#ifndef ICCC_WORKS
  "Panner*NumberDialog_popup.width :		170",
  "Panner*NumberDialog_popup.height :		130",
  "Panner*NumberDialog.width :			170",
  "Panner*NumberDialog.height :			130",
  "Panner*AddDisplayDialog_popup.width :	170",
  "Panner*AddDisplayDialog_popup.height :	130",
  "Panner*AddDisplayDialog.width :		170",
  "Panner*AddDisplayDialog.height :		130",
  "Panner*HelpDialog_popup.width :		325",
  "Panner*HelpDialog_popup.height :		155",
  "Panner*HelpDialog.width :			325",
  "Panner*HelpDialog.height :			155",
#endif
  "Panner*canvas.background:		grey40",
  "Panner*canvas.foreground:		yellow",
  /* Menu entry definitions */
  "Panner*cascade1.labelString:		File",
  "Panner*cascade2.labelString:		Display",
  "Panner*cascade3.labelString:		Settings",
  "Panner*cascade4.labelString:		Help",
  "Panner*b1.labelString:		Quit",
  "Panner*b2.labelString:		Update",
  "Panner*b3.labelString:		New Display",
#ifdef ADD_PAN
  "Panner*b4.labelString:		Start Edge",
  "Panner*b5.labelString:		Stop  Edge",
  "Panner*b6.labelString:		Edge Speed",
  "Panner*b7.labelString:		Edge Frequency",
  "Panner*b8.labelString:		Edge Slide",
  "Panner*b9.labelString:		Edge Multiple",
  "Panner*b10.labelString:		Pan  Multiple",

  "Panner*NumberDialog.selectionLabelString: Number:",
#endif
  "Panner*AddDisplayDialog*selectionLabelString: Display name:",

  "Panner*HelpDialog*messageString:\
Panner Demo\\n-----------\\n\
Grab or click Button1 to pan display.\\n\
Grab Button3 to drag an individual window.\\n\
Use 'New Display...' to view another display.",

  "Panner*WarningDialog*messageString:\
The panner window is not pinned!\\n\
Add the line:\\n\\n    \"Mwm*Panner*ClientPinned: True\"\\n\\n\
to your .Xdefaults file and restart Mwm.",

  NULL
};

typedef struct _PannerInfoRec
{
  Display *display;
  Screen *screen;
  Widget shell;
  Widget utmShell;              /* drawing area used for UTM */
  Widget canvas;
  int thumbX, thumbY;
  unsigned int thumbW, thumbH;  /* required for 1st XOR dash line and etc */
  unsigned int canvasW, canvasH;
  int lastEventX, lastEventY;
#ifdef ADD_PAN
  int oldEventX, oldEventY;     /* required for 2nd XOR dash line */
  int screenW, screenH;         /* only used or updated for LOCAL */
#endif
  Atom WM;                      /* need selections on the correct display */
  Atom WM_PAN;
  Atom WM_GOTO;
  Atom WM_PAN_POS;
} PannerInfoRec;

typedef struct _PanPostion
{
  long x;
  long y;
} PanPosition;

#ifdef ADD_PAN
typedef enum
{ MENU_QUIT, MENU_UPDATE, MENU_NEW, MENU_HELP, EDGE_START, EDGE_STOP,
  EDGE_SPEED, EDGE_FREQ, EDGE_SLIDE, EDGE_MULT, PAN_MULT
} MenuFunction;
#else
typedef enum
{ MENU_QUIT, MENU_UPDATE, MENU_NEW, MENU_HELP } MenuFunction;
#endif
typedef enum
{ UNKNOWN, VERIFYING, VERIFIED } PinState;

/*
 * globals.
 */
XtAppContext app;
unsigned short DSP;             /* index of active display */
Widget notebook;
GC thumbGC, canvasGC;
XContext context;
PannerInfoRec *pInfoList;
unsigned long cells[COLOR_COUNT];
#ifndef ADD_PAN
PinState pinnedState = UNKNOWN;
#endif
int origX, origY;

#define DPY_LOC		pInfoList[LOCAL].display
#define DPY_ACT		pInfoList[DSP].display
#define SCR_LOC		pInfoList[LOCAL].screen
#define SCR_ACT		pInfoList[DSP].screen
#define SCR_LOCW	pInfoList[LOCAL].screenW
#define SCR_LOCH	pInfoList[LOCAL].screenH
#define SCR_ACTW	pInfoList[DSP].screenW
#define SCR_ACTH	pInfoList[DSP].screenH

/* #ifdef X_ONLY use x_only */
Boolean LOCK = False;




static void OpenNewDisplay (String, Widget, PannerInfoRec *);
static void UpdatePannerCB (Widget, XtPointer, XtPointer);
static void ChangePageCB (Widget, XtPointer, XtPointer);
static void DoAddDisplayCB (Widget, XtPointer, XtPointer);
#ifdef ADD_PAN
static void DoNumEdgeCB (Widget, XtPointer, XtPointer);
#endif
static void DestinationCB (Widget, XtPointer, XtPointer);
#ifdef TRY_ALLOC_ON_MULTISCREENS
static void DoneMoveScreenCB (Widget, XtPointer, XtPointer);
#endif
#ifndef ADD_PAN
static void WatchForWindowPanning (Display * dsp);
#endif
static void HandlePropertyChange (XEvent * event);
static void UpdatePannerView (PannerInfoRec * pInfoList, int remoteDsp);
static void DrawWindows (PannerInfoRec *);
static void DrawThumb (PannerInfoRec *);
static void SetupColorsAndGCs ();
static GC GetXorGC (Widget);
static GC GetCanvasGC (Widget);
#ifndef ADD_PAN
static void SetWindowColor (PannerInfoRec *, int);
#endif
static void TranslateCoordinates (PannerInfoRec *,
                                  int, int, unsigned int, unsigned int,
                                  int *, int *, unsigned int *,
                                  unsigned int *);
static int IgnoreError (Display * dsp, XErrorEvent * event);
static void StartTracking (Widget, XtPointer, XEvent *, Boolean *);
static void DoTracking (Widget, XtPointer, XEvent *, Boolean *);
static void StopTracking (Widget, XtPointer, XEvent *, Boolean *);
static void MoveScreen (PannerInfoRec *, int, int, Time);

static XtPointer PackCARD32 (XtPointer, CARD32);
static XtPointer PackCARD16 (XtPointer, CARD16);
static XtPointer PackCARD8 (XtPointer, CARD8);

static void CreateMenuBar (Widget parent);
static void MenuCB (Widget w, XtPointer clientData, XtPointer callData);
static void DoUpdatePanner ();
static void DoAddDisplay ();
#ifdef ADD_PAN
static void DoNumEdge ();
#endif
static void DoHelp ();
static void DoQuit ();
#ifndef ADD_PAN
static void CheckPinnedState ();
#endif
static void ShowPinStateWarning ();
static void HandleInitialExpose (Widget, XtPointer, XEvent *, Boolean *);
static Time GetTimestamp (Display * dsp);

#ifdef ADD_PAN
static void DoStartEdge (XtPointer);
static void DoStopEdge (XtPointer);
#endif

/* ---   ADD_PAN   --- */

#ifdef ADD_PAN
#define ADD_COLOR
/* track_update how far button3 drag box moves before refresh
   because refresh is "blinky" , 1 out of 10 should reduce flashing
 */
#define PAN_MAX_SPEED		10000
static Boolean x_only = False, button3 = False, tracking = False;
static Boolean panedge = False, centering=False;
static int track_count = 0;
static int init_screen_width = INIT_SCREEN_WIDTH;
static void DrawThumbSmall (PannerInfoRec *);
/* GC thumbGCSmall, extraGC; */
#define thumbGCSmall thumbGC
static void panEdge (XEvent *);
static long which_num = 0;
static int edge_cnt = 0;
static XtIntervalId interval_id;
static float edge_update = 10;
static float edge_speed = 48;
static float edge_freq = 100;
static float edge_slide = 1;
static float b3_upd_freq = 10;
/* 0 Xme grabs many small dy/xy (smooth) before a person has the
 * chance to move the mouse quicker - causing a mandatory second or two of
 * slow panning that is "un-responsive", ignore every other mouse update helps
 */
static float pan_mult = 2;
static float edge_mult = 1;
static XEvent time_event;
/* defauls are for sluggish update for sake of bandwidth defaults
 * the methods are more hoggish the higher the freq
 */
#endif

/* ---  Not an ADD_PAN related --- */

#ifdef ADD_COLOR
static char color_fail = 0;
#endif

#ifdef TRY_ALLOC_ON_MULTISCREENS
/* found enver freed bug X_ONLY */
static int msg_xallocs = 0;
#else
/*
 * note XtPointer is void* , typeless
 * static long fulldata [(6+1)*4+1];
 * 100, rotation, is our memory of how many we can append before replacing
 */
#ifdef int32_t
static int32_t data[PAN_ELEM];
#else
#warning "no int32_t, assuming long is at least 32bit"
static long data[PAN_ELEM];
#endif
static XtPointer fulldata = data;
#endif

#ifdef ADD_COLOR
#define NUMBER_OF_COLORS 100    /* recycle if over 100 windows on dektop */
static unsigned long colorw[NUMBER_OF_COLORS + 1];
#endif





/*----------------------------------------------------------------*
 |                             main                               |
 *----------------------------------------------------------------*/
int
main (int argc, char **argv)
{
  Widget mainWin, frame;
  XEvent event;

#ifdef ADD_PAN
  int i;
  char *p;
  for (i = 1; i < argc; ++i)
  {
    if (strcmp (argv[i], "--help") == 0)
    {
      printf
        ("panner  [--help --version --x-only --VERFIED --color-fail --screen-width= --edge-freq= --edge-update= --edge-slide= --b3_upd_freq=] --pan-mult= --b3-quirk...\n");
      return 0;
    };
    if (strcmp (argv[i], "--version") == 0)
    {
      printf ("panner 1.1\n");
      return 0;
    };
    if (strcmp (argv[i], "--x-only") == 0)
      x_only = True;
    if (strcmp (argv[i], "--color-fail") == 0)
      color_fail = 1;
    if (strncmp (argv[i], "--screen-width=", 15) == 0)
    {
      p = argv[i] + 15;
      init_screen_width = atoi (p);
    }
    if (strncmp (argv[i], "--edge-update=", 14) == 0)
    {
      p = argv[i] + 14;
      edge_update = atof (p);
    }
    if (strncmp (argv[i], "--b3-upd-freq=", 14) == 0)
    {
      p = argv[i] + 14;
      b3_upd_freq = atof (p);
    }
    if (strncmp (argv[i], "--edge-freq=", 12) == 0)
    {
      p = argv[i] + 12;
      edge_freq = atof (p);
    }
    if (strncmp (argv[i], "--edge-slide=", 13) == 0)
    {
      p = argv[i] + 13;
      edge_slide = atof (p);
    }
    if (strncmp (argv[i], "--edge-speed=", 13) == 0)
    {
      p = argv[i] + 13;
      edge_speed = atof (p);
    }
    if (strncmp (argv[i], "--pan-mult=", 11) == 0)
    {
      p = argv[i] + 11;
      pan_mult = atof (p);
    }
    if (strncmp (argv[i], "--edge-mult=", 12) == 0)
    {
      p = argv[i] + 12;
      edge_mult = atof (p);
    }
  }
#endif

#if defined(ADD_PAN) && defined(PAN_DEBUG)
  fprintf (stderr, "\npanner: x-only=%d color-fail=%d screen-width=%d\n\
edge_update=%f edge_freq=%f ege_slide=%f edge_speed=%f pan_mult=%f edge_mult=%d\n",
x_only, color_fail, init_screen_width,
edge_update, edge_freq, edge_slide, edge_speed,pan_mult,edge_mult);
#endif

  pInfoList = (PannerInfoRec *) XtMalloc (sizeof (PannerInfoRec) *
                                          MAX_DISPLAY_COUNT);

  if ((pInfoList == (PannerInfoRec *) NULL))
  {
    fprintf (stderr, "Xt out of memory\n");
    exit (1);
  }
  for (DSP = 0; DSP < MAX_DISPLAY_COUNT; DSP++)
    DPY_ACT = NULL;
  DSP = LOCAL;

#ifdef ADD_PAN
  /* motif-2.3.4 has this - new X manpages suggest - likely same here though */
  pInfoList[LOCAL].shell = XtVaOpenApplication (&app, "Panner", NULL,
                                                0, &argc, argv,
                                                fallback,
                                                sessionShellWidgetClass,
                                                NULL);
#else
  pInfoList[LOCAL].shell = XtVaAppInitialize (&app, "Panner", NULL, 0,
                                              &argc, argv, fallback, NULL);
#endif

  DPY_LOC = XtDisplay (pInfoList[LOCAL].shell);


  mainWin = XmCreateMainWindow (pInfoList[LOCAL].shell, "mainWin", NULL, 0);
  XtManageChild (mainWin);
  CreateMenuBar (mainWin);

  frame = XtVaCreateManagedWidget ("frame", xmFrameWidgetClass, mainWin,
                                   NULL);
  notebook =
    XtVaCreateManagedWidget ("notebook", xmNotebookWidgetClass, frame, NULL);

  XtRealizeWidget (pInfoList[LOCAL].shell);

  context = XUniqueContext ();
  OpenNewDisplay ( /*$DISPLAY */ NULL, notebook, pInfoList);

  SetupColorsAndGCs ();

  XtAddCallback (notebook, XmNpageChangedCallback, ChangePageCB, pInfoList);
  XtAddEventHandler (notebook, ExposureMask, False, HandleInitialExpose,
                     NULL);

  XtMapWidget (pInfoList[LOCAL].shell);

  SCR_LOCW = WidthOfScreen (SCR_LOC);
  SCR_LOCH = HeightOfScreen (SCR_LOC);

  memset (&time_event, 0, sizeof (time_event));

  /* XtAppMainLoop (app); */

#ifdef ADD_PAN
  for (;;)
  {
#if 0
    /*
     * event driven panEdge would be right but require XkbSelectEvents to work
     * which is useless do Alt-> if already pressing Alt (Mwm does keys)
     * this works but if ie,firefox is on edge it doesnt activate over firefox
     * (fvwm ran into same problem variously, src says, it has virt win to use)
     *
     * XkbSelectEvents(Dpy,XkbUseCoreKbd,XkbStateNotifyMask,XkbStateNotifyMask)
     * can get key events wherever focus is: but XSelectInput cannot.
     * apps consume "unused" mouse events (xterm no, but most do)
     *
     * another problem is adding complexity could cause Input headaches
     */
    if (panedge && !button3)
    {
      /* requires XPutBackEvent: if( XWindowEvent( .. PointerMotionMask .. ) */
      if (DPY_ACT != NULL && XPeekEvent (DPY_ACT, &event))
      {
        /* panEdge checks bounds too */
        if (event.type == MotionNotify)
          panEdge (&event);
        /* no good threhold for stopping starting , want refresh anyway */
        if ((++edge_cnt % edge_update) == 0)
          UpdatePannerView (pInfoList, DSP);
      }
    }
#endif /* 0 */

    XtAppNextEvent (app, &event);

    if (event.type == PropertyNotify)
      HandlePropertyChange (&event);

    XtDispatchEvent (&event);

  }
#endif /* ! ADD_PAN */
}


/*----------------------------------------------------------------*
 |                        OpenNewDisplay                          |
 | - Get new display connection to named display.                 |
 | - If display name is not NULL, create shell on the display.    |
 | - Fill in correct record in pInfoList.                         |
 *----------------------------------------------------------------*/
static void
OpenNewDisplay (String displayName,
                Widget notebook, PannerInfoRec * pInfoList)
{
  int newDsp = 0;
  int argc = 0;
  char **argv = NULL;
  Dimension canvasW, canvasH;
  char selectionName[40];
  PannerInfoRec *pInfo;
  Widget tab;
  XmString tabName;
  XtCallbackList cbList;


  /*
   * If NULL, then the display's already been created.
   */
  if (displayName != NULL)
  {
    XtVaGetValues (notebook, XmNlastPageNumber, &newDsp,
                   XmNpageChangedCallback, &cbList, NULL);
    newDsp++;

    if ((pInfoList[newDsp].display = XOpenDisplay (displayName)) == NULL)
    {
      fprintf (stderr, "ERROR - Can't open display \"%s\".\n", displayName);
      return;
    }

    XtDisplayInitialize (app, pInfoList[newDsp].display, "panner", "Panner",
                         NULL, 0, &argc, argv);

    /* create an unmapped shell on the remote display */
    pInfoList[newDsp].shell =
      XtVaAppCreateShell ("panner", "Panner", applicationShellWidgetClass,
                          pInfoList[newDsp].display,
                          XmNmappedWhenManaged, False, NULL);

    XtRealizeWidget (pInfoList[newDsp].shell);
  }
  /*
   * For UTM to work, there must be a drawing area or UTM saavy
   * widget.
   *
   *  We must set-up a destination callback function that
   *  does the actual transfer of the parameter info to Mwm.
   */

  pInfoList[newDsp].utmShell
    = XtVaCreateManagedWidget ("utmShell", xmDrawingAreaWidgetClass,
                               pInfoList[newDsp].shell,
                               XmNmappedWhenManaged, False, NULL);

  XtAddCallback (pInfoList[newDsp].utmShell, XmNdestinationCallback,
                 DestinationCB, &(pInfoList[newDsp]));

  /*
   * Initialize the correct record in the pInfoList.
   */
  pInfoList[newDsp].screen = XtScreen (pInfoList[newDsp].shell);


  /*
   * setup handler to watch when Mwm changes the root property.
   * first store some data on the root window.
   */
#ifndef ADD_PAN
  XSaveContext (pInfoList[newDsp].display, DefaultRootWindow (pInfoList[newDsp].display), context, (XPointer) (long) newDsp); /* store index into panner info. */
  WatchForWindowPanning (pInfoList[newDsp].display);
#endif

  /*
   * Add another page to the notebook.
   * First must set size correctly.
   */

  XtVaGetValues (pInfoList[LOCAL].shell,
                 XmNwidth, &canvasW, XmNheight, &canvasH, NULL);
  pInfoList[newDsp].canvas
    = XtVaCreateManagedWidget ("canvas", xmDrawingAreaWidgetClass, notebook,
                               XmNchildType, XmPAGE,
                               XmNpageNumber, newDsp,
                               XmNwidth, canvasW, XmNheight, canvasH, NULL);
  XtAddCallback (pInfoList[newDsp].canvas, XmNexposeCallback, UpdatePannerCB,
                 pInfoList);

  pInfoList[LOCAL].canvasW = canvasW;
  pInfoList[LOCAL].canvasH = canvasH;

  if (displayName == NULL)
    tabName = XmStringCreate ("LOCAL", XmFONTLIST_DEFAULT_TAG);
  else
    tabName = XmStringCreate (displayName, XmFONTLIST_DEFAULT_TAG);
  tab = XtVaCreateManagedWidget ("tab", xmPushButtonWidgetClass, notebook,
                                 XmNlabelString, tabName,
                                 XmNchildType, XmMAJOR_TAB, NULL);
  XmStringFree (tabName);

#ifdef ADD_PAN
  pInfoList[newDsp].thumbW = init_screen_width;
#else
  pInfoList[newDsp].thumbW = INIT_SCREEN_WIDTH;
#endif

  pInfoList[newDsp].screenW = WidthOfScreen (pInfoList[newDsp].screen);
  pInfoList[newDsp].screenH = HeightOfScreen (pInfoList[newDsp].screen);

  pInfoList[newDsp].thumbH =
    (float) pInfoList[newDsp].thumbW *
    (float) pInfoList[newDsp].screenH / (float) pInfoList[newDsp].screenW;

  XtVaGetValues (pInfoList[newDsp].canvas, XmNwidth, &canvasW,
                 XmNheight, &canvasH, NULL);
  pInfoList[newDsp].thumbX =
    (float) canvasW / (float) 2 - (float) pInfoList[newDsp].thumbW / (float) 2;
  pInfoList[newDsp].thumbY =
    (float) canvasH / (float) 2 - (float) pInfoList[newDsp].thumbH / (float) 2;

  pInfoList[newDsp].canvasW = canvasW;
  pInfoList[newDsp].canvasH = canvasH;

  /* Setup the atoms needed to communicate with Mwm. Check screen number! */
  sprintf (selectionName, WM_SELECTION_FORMAT,
           XScreenNumberOfScreen (pInfoList[newDsp].screen));
  pInfoList[newDsp].WM = XmInternAtom (pInfoList[newDsp].display,
                                       selectionName, False);
  pInfoList[newDsp].WM_PAN = XmInternAtom (pInfoList[newDsp].display,
                                           "_MOTIF_WM_PAN", False);
  pInfoList[newDsp].WM_GOTO = XmInternAtom (pInfoList[newDsp].display,
                                            "_MOTIF_WM_GOTO", False);
  pInfoList[newDsp].WM_PAN_POS = XmInternAtom (pInfoList[newDsp].display,
                                               "_MOTIF_WM_PAN_POSITION",
                                               False);

  XtAddEventHandler (pInfoList[newDsp].canvas, ButtonPressMask, False,
                     StartTracking, (XtPointer) & pInfoList[newDsp]);
}



/*========================== CALLBACKS ==========================*/

/*----------------------------------------------------------------*
 |                        UpdatePannerCB                          |
 *----------------------------------------------------------------*/
static void
UpdatePannerCB (Widget w, XtPointer clientData, XtPointer callData)
{
  XmDrawingAreaCallbackStruct *cb = (XmDrawingAreaCallbackStruct *) callData;
  PannerInfoRec *pInfoList = (PannerInfoRec *) clientData;

  if (cb->reason == XmCR_EXPOSE)
  {
    XExposeEvent *event = (XExposeEvent *) cb->event;

    /* Last expose event received - do the drawing. */
    if (event->count == 0)
      UpdatePannerView (pInfoList, DSP);
  }

  else
    /*
     * Update button pressed.
     */
    UpdatePannerView (pInfoList, DSP);
}


/*----------------------------------------------------------------*
 |                         ChangePageCB                           |
 | Invoked just prior to notebook page change. Any drawing here   |
 | would be lost.                                                 |
 *----------------------------------------------------------------*/
static void
ChangePageCB (Widget w, XtPointer clientData, XtPointer callData)
{
  PannerInfoRec *pInfoList = (PannerInfoRec *) clientData;
  XmNotebookCallbackStruct *nbData = (XmNotebookCallbackStruct *) callData;
  int pageNumber;

  pageNumber = nbData->page_number;
  if ((pageNumber >= MAX_DISPLAY_COUNT) ||
      (pInfoList[pageNumber].display == NULL))
  {
    fprintf (stderr, "ERROR - bad display index. (%d).\n", pageNumber);
  }
  else
  {
    DSP = pageNumber;
  }
}


/*----------------------------------------------------------------*
 |                        DoAddDisplayCB                          |
 *----------------------------------------------------------------*/
static void
DoAddDisplayCB (Widget w, XtPointer clientData, XtPointer callData)
{
  XmSelectionBoxCallbackStruct *cb =
    (XmSelectionBoxCallbackStruct *) callData;
  PannerInfoRec *pInfoList = (PannerInfoRec *) clientData;
  char *dspName;                /* Free when done. */
  String appName, appClass;     /* Don't free - owned by Xt. */

  /* should not happen unless a bug or type is in code */
  if ((pInfoList == (PannerInfoRec *) NULL))
    return;

  XtGetApplicationNameAndClass (DPY_LOC, &appName, &appClass);
  XmStringGetLtoR (cb->value, XmSTRING_DEFAULT_CHARSET, &dspName);

  if (dspName && strlen(dspName) != 0)
    OpenNewDisplay (dspName, notebook, pInfoList);
  if (dspName)
    XtFree (dspName);
}

#ifdef ADD_PAN

/*----------------------------------------------------------------*
 |                        DoNumEdgeCB                             |
 *----------------------------------------------------------------*/

static void
DoNumEdgeCB (Widget w, XtPointer clientData, XtPointer callData)
{
  XmSelectionBoxCallbackStruct *cb =
    (XmSelectionBoxCallbackStruct *) callData;
  PannerInfoRec *pInfoList = (PannerInfoRec *) clientData;
  char *dspName;                /* Free when done. */
  String appName, appClass;     /* Don't free - owned by Xt. */
  float f;

  if ((pInfoList == (PannerInfoRec *) NULL))
    return;

  XtGetApplicationNameAndClass (DPY_LOC, &appName, &appClass);
  XmStringGetLtoR (cb->value, XmSTRING_DEFAULT_CHARSET, &dspName);

  if( dspName && strlen(dspName) != 0 )
  {
    f = atof (dspName);
    switch (which_num)
    {
    case EDGE_SPEED:
      edge_speed = f;
      break;
    case EDGE_FREQ:
      edge_freq = f;
      break;
    case EDGE_SLIDE:
      edge_slide = f;
      break;
    case PAN_MULT:
      pan_mult = f;
      break;
    case EDGE_MULT:
      edge_mult = f;
      break;
    default:
      fprintf(stderr,"NumberDialog error\n");
      break;
    }
  }

  if (dspName)
    XtFree (dspName);
}

#endif

/*----------------------------------------------------------------*
 |                         DestinationCB                          |
 | This function gets invoked by UTM when a sink has been estab-  |
 | lished and a request initiated against another selection.  The |
 | purpose here is to set-up the parameters and pass them to the  |
 | owner of the selection.  The parameter data has already been   |
 | allocated in the MoveScreen() function.                        |
 | clientData holds the pannerInfoRec corresponding to the right  |
 | display.                                                       |
 *----------------------------------------------------------------*/
static void
DestinationCB (Widget w, XtPointer clientData, XtPointer callData)
{
  XmDestinationCallbackStruct *dcs = (XmDestinationCallbackStruct *) callData;
  PannerInfoRec *pInfo = (PannerInfoRec *) clientData;
  Atom target;

  /*
   * Pass the data to free in the clientData field.
   * location_data points to the param data. This was set in
   * MoveScreen().
   */

  /* FIRST - setup the parameters to pass. */
  XmTransferSetParameters (dcs->transfer_id, dcs->location_data,  /* pointer to param data. */
                           PAN_FORMAT, PAN_ELEM,  /* should be calculated. */
                           dcs->selection); /* type - don't care. */

  /* LAST - Make the transfer. */
  XmTransferValue (dcs->transfer_id, pInfo->WM_PAN, /* target for conversion. */
#ifdef ADD_PAN
                   NULL,        /* CB proc invoked when done. */
#else
                   DoneMoveScreenCB,  /* CB proc invoked when done. */
#endif
                   dcs->location_data,  /* clientData - to be freed. */
                   dcs->time);
}

/*
 * utm uses an (empty) window Motif creates which exists as a sink for
 * certain send/recv events.  i'm unsure what more utm does.  Xm XmeNamedSink ?
 */



/*----------------------------------------------------------------*
 |                       DoneMoveScreenCB                         |
 *----------------------------------------------------------------*/

#ifndef ADD_PAN

static void
DoneMoveScreenCB (Widget w, XtPointer clientData, XtPointer callData)
{
  /*
   * Conversion completed. Safe to free param data.
   */

  /* see notes at EOF */

#ifdef TRY_ALLOC_ON_MULTISCREENS
  XtFree ((char *) clientData);
  --msg_xallocs;
#if defined(ADD_PAN) && defined(PAN_DEBUG)
  fprintf (stderr, "allocs=%d\n", msg_xallocs);
  fflush (stderr);
#endif
#endif

}
#endif /* ADD_PAN */





/*=========================== PAN-HANDLING ==========================*/


/*----------------------------------------------------------------*
 |                     WatchForWindowPanning                      |
 *----------------------------------------------------------------*/
#ifndef ADD_PAN
static void
WatchForWindowPanning (Display * dsp)
{
  XWindowAttributes attr;
  Window rwin = DefaultRootWindow (dsp);

  if (!dsp)
    return;

  /* Watch whenever the window manager's panning position changes. */
  /* Mwm stores the position in properties on the root window.     */
  /* This is stored in the _MOTIF_WM_PAN_POSITION property.        */

  XGetWindowAttributes (dsp, rwin, &attr);

  if (!(attr.your_event_mask & PropertyChangeMask))
    XSelectInput (dsp, rwin, attr.your_event_mask | PropertyChangeMask);
}
#endif


/*----------------------------------------------------------------*
 |                     HandlePropertyChange                       |
 | This routine checks the property changed and if its the right  |
 | property, grab the new panning position.                       |
 *----------------------------------------------------------------*/

/* 
 * for ADD_PAN this always exits before doing anything
 * called only at startup (due to startup)
 */

static void
HandlePropertyChange (XEvent * event)
{
  XPropertyEvent *propEvent = (XPropertyEvent *) event;
  int iDsp;

  /* Get the correct info record stored with the context manager. */
  if (XFindContext (propEvent->display, propEvent->window, context,
                    (XPointer *) & iDsp))
    return;

  /* should not happen unless a bug or type is in code */
  if ((pInfoList == (PannerInfoRec *) NULL) || (DPY_LOC == NULL))
    return;

#ifdef ADD_PAN
  /*
   * infrequently here - probably a resize - nice time to update canvasW
   * and size and such
   */
  if (iDsp == DSP || iDsp == LOCAL)
  {
#ifdef PAN_DEBUG
    fprintf (stderr, "prop\n");
#endif
    DoUpdatePanner ();
  }
#endif

#ifndef ADD_PAN
  /* check if this is the right one. Othersize, we'll update when another
   * display changes.
   */
  if (propEvent && propEvent->atom != pInfoList[iDsp].WM_PAN_POS)
    return;

  /* if the display doesn't match the current one, then punt. */
  if (iDsp == DSP)
  {
    if (pinnedState == VERIFIED)
      UpdatePannerView (pInfoList, iDsp);
    else
    {
      Window rWin, child;
      int x, y, newX, newY;
      unsigned int width, height, bWidth, depth;

      /* Get position of the top-level shell */
      XGetGeometry (DPY_LOC,
                    XtWindow (pInfoList[LOCAL].shell), &rWin, &x, &y, &width,
                    &height, &bWidth, &depth);

      XTranslateCoordinates (DPY_LOC,
                             XtWindow (pInfoList[LOCAL].shell),
                             rWin, x, y, &newX, &newY, &child);
      if ((newX == origX) && (newY == origY))
        pinnedState = VERIFIED;
      else
        ShowPinStateWarning ();
    }
  }
#endif

#ifdef ADD_PAN
  /* ADD_PAN resets per MoveDesktop anyhow see below */
  if (x_only)
    LOCK = False;
#else
#ifdef X_ONLY
  /* ? ADD_PAN is BROKEN for X_ONLY it never gets WM_foo to Un LOCK */
  LOCK = False;
#endif
#endif

}



/*============================ DRAWING ===========================*/

/*----------------------------------------------------------------*
 |                         UpdatePannerView                       |
 *----------------------------------------------------------------*/
static void
UpdatePannerView (PannerInfoRec * pInfoList, int remoteDsp)
{
  /* should not happen unless a bug or type is in code */
  if ((pInfoList == (PannerInfoRec *) NULL)
      || (pInfoList[remoteDsp].canvas == NULL))
    return;

#ifndef ADD_PAN
  XClearArea (DPY_LOC,
              XtWindow (pInfoList[remoteDsp].canvas), 0, 0, 0, 0, False);
#endif

  DrawWindows (pInfoList);
  DrawThumb (&pInfoList[remoteDsp]);
}


/*----------------------------------------------------------------*
 |                          DrawWindows                           |
 *----------------------------------------------------------------*/

#define PAN_XI

static void
DrawWindows (PannerInfoRec * pInfoList)
{
  Window realRoot, root, parent, win, *child = NULL;
  XWindowAttributes attr;
  int i, x, y, c_i;
  unsigned int childCount, width, height;
  int (*oldHandler) ();

#if defined(ADD_PAN) && defined(PAN_XI)
  XWindowAttributes cattr;
  XGCValues values;
  XSetWindowAttributes xswa;
  Window tmpwin, realRoot2, win2;
  Pixmap pixmap;
  int once;
#endif

  /* should not happen unless a bug or type is in code */
  if ((pInfoList == (PannerInfoRec *) NULL) || (DPY_ACT == NULL))
    return;

  realRoot = RootWindow (DPY_ACT, XScreenNumberOfScreen (SCR_ACT));
#if defined(ADD_PAN) && defined(PAN_XI)
  realRoot2 = RootWindow (DPY_LOC, XScreenNumberOfScreen (SCR_LOC));
#endif

  if (!XQueryTree (DPY_ACT, realRoot, &root, &parent, &child, &childCount))
    return;

  win = XtWindow (pInfoList[LOCAL].canvas),
#if defined(ADD_PAN) && defined(PAN_XI)
    win2 = XtWindow (pInfoList[DSP].canvas),
    /*
     * pixmap is on server, drawing is created on server, client says how to draw
     * user sees flashing if drawing is on mapped visible win (the canvas)
     * to have spare and invisilbe pixmap must have a unmapped win: create it
     * (Xt easier? would map and manage it - unsure how to Xt prevent that)
     */
    pixmap = (Pixmap) NULL;
  XGetWindowAttributes (DPY_LOC, win, &cattr);
  XtVaGetValues (pInfoList[LOCAL].canvas, XmNforeground, &values.foreground,
                 XmNbackground, &values.background, NULL);
  memset (&xswa, 0, sizeof (xswa));
  xswa.background_pixmap = ParentRelative;
  xswa.background_pixel = values.background;
  xswa.border_pixel = 0;
  xswa.bit_gravity = 0;
  xswa.win_gravity = 0;
  xswa.backing_planes = 0;
  xswa.backing_pixel = 0;
  xswa.save_under = False;
  xswa.event_mask = 0;
  xswa.do_not_propagate_mask = True;
  xswa.override_redirect = True;
  xswa.colormap = DefaultColormapOfScreen (SCR_LOC);
  xswa.cursor = None;
  /* let prog exit if it fails */
  tmpwin = XCreateWindow (DPY_LOC,
                          XtWindow (notebook),
                          0, 0,
                          cattr.width, cattr.height, 0,
                          CopyFromParent, CopyFromParent, CopyFromParent,
                          0, &xswa);
  pixmap =
    XCreatePixmap (DPY_LOC, realRoot2, cattr.width, cattr.height,
                   cattr.depth);
  XSetForeground (DPY_LOC, canvasGC, values.background);
  XFillRectangle (DPY_LOC, pixmap, canvasGC, 0, 0, cattr.width, cattr.height);
#endif
  /* loop XQueryTree return , drawing on pixmap as we go */

  /* 
   * We need to install an error handler since the window-tree may
   * become invalid while where still processing the list.
   */
  oldHandler = XSetErrorHandler (IgnoreError);
  once = 0;
  c_i = 0;
  for (i = 0; i < childCount; i++)
  {
    XGetWindowAttributes (DPY_ACT, child[i], &attr);
    if (attr.map_state == IsViewable)
    {
      once = 1;
      TranslateCoordinates (&pInfoList[DSP],
                            attr.x, attr.y, attr.width, attr.height,
                            &x, &y, &width, &height);
#ifdef ADD_COLOR
      if (color_fail == 2)
        XSetForeground (DPY_LOC, canvasGC, colorw[++c_i % NUMBER_OF_COLORS]);
      else
#endif
        XSetForeground (DPY_LOC, canvasGC, cells[(++c_i + 1) % COLOR_COUNT]);
#if defined(ADD_PAN) && defined(PAN_XI)
      XFillRectangle (DPY_LOC, pixmap, canvasGC, x, y, width, height);
#else
      XFillRectangle (DPY_LOC, win2, canvasGC, x, y, width, height);
#endif
    }
  }
#if defined(ADD_PAN) && defined(PAN_XI)
  if (once)
    XCopyArea (DPY_LOC, pixmap, win2, canvasGC, 0, 0, cattr.width,
               cattr.height, 0, 0);
  XFreePixmap (DPY_LOC, pixmap);
  XDestroyWindow (DPY_LOC, tmpwin);
#endif

  XSetErrorHandler (oldHandler);

  if (child != NULL)
    XFree ((char *) child);
}

/* could keep pixmap to update damage rect - not necessary - is xi still valid?
 *
 * proabably the above could use stippling or translucense but doing so
 * uncarefully would quickly lead to confusing or ugly graphics
 */


/*----------------------------------------------------------------*
 |                          DrawThumb                             |
 *----------------------------------------------------------------*/
static void
DrawThumb (PannerInfoRec * pInfo)
{
  /* should not happen unless a bug or type is in code */
  if (!pInfo)
    return;
  XDrawRectangle (XtDisplay (pInfo->canvas), XtWindow (pInfo->canvas),
                  thumbGC, pInfo->thumbX, pInfo->thumbY,
                  pInfo->thumbW, pInfo->thumbH);
}

static void
DrawThumbSmall (PannerInfoRec * pInfo)
{
  /* should not happen unless a bug or type is in code */
  if (!pInfo)
    return;
  XDrawRectangle (XtDisplay (pInfo->canvas), XtWindow (pInfo->canvas),
                  thumbGCSmall,
                  (float) pInfo->oldEventX - (float) pInfo->thumbW / (float) 8,
                  (float) pInfo->oldEventY - (float) pInfo->thumbH / (float) 8,
                  (float) pInfo->thumbW / (float) 4, (float) pInfo->thumbH / (float) 4);
}


/*----------------------------------------------------------------*
 |                       SetupColorsAndGCs                        |
 | Called once at the beginning to setup some drawing stuff.      |
 *----------------------------------------------------------------*/
static void
SetupColorsAndGCs ()
{
  int i;
  XColor color;
  Colormap cmap;

  /* should not happen unless a bug or type is in code */
  if ((pInfoList == (PannerInfoRec *) NULL)
      || (pInfoList[LOCAL].canvas == NULL))
    return;

  cmap = DefaultColormapOfScreen (SCR_LOC);

  /*
   * set-up the global GCs.
   */
#if defined(ADD_PAN) && 0
  thumbGCSmall = GetXorGC (pInfoList[LOCAL].canvas);
  extraGC = GetCanvasGC (pInfoList[LOCAL].canvas);
#endif
  thumbGC = GetXorGC (pInfoList[LOCAL].canvas);
  canvasGC = GetCanvasGC (pInfoList[LOCAL].canvas);

  srand (time (NULL));

  /*
   * Allocate the global color cells for the drawing of each windows.
   * The more random the colors, the better.
   */

  if (XAllocColorCells (DPY_LOC, cmap, False, NULL, 0, cells, COLOR_COUNT))
  {
    for (i = 0; i < COLOR_COUNT; i++)
    {
      color.red = rand () % 65535;
      color.blue = rand () % 65535;
      color.green = rand () % 65535;
      color.pixel = cells[i];
      XStoreColor (DPY_LOC, cmap, &color);
    }
  }
#ifdef ADD_COLOR
  else
  {
    int i, max;
    unsigned long color;
    color_fail |= 2;
    int depth;                  /* ie, 16bit */
    float fmul;
    XGCValues values;
    fprintf (stderr, "panner: no color map, will use depth values\n");
    XtVaGetValues (pInfoList[LOCAL].canvas, XmNforeground, &values.foreground,
                   XmNbackground, &values.background, NULL);
    /* make random color, tuned to screen depth */
    depth = DefaultDepth (DPY_LOC, DefaultScreen (DPY_LOC));
    if (depth == 0)
      depth = 8;                /* err ok */
    depth = pow (2, depth);
    fmul = (float) depth / (float) RAND_MAX;
    if (fmul == 0)
      fmul = 1;
    for (i = 0; i < NUMBER_OF_COLORS; ++i)
    {
      max=10000;
      while ( --max )
      {
        color = (unsigned long) (float) rand () * fmul;
        if( color != values.background )
          break;
      }
      colorw[i] = color;
    }
  }
#endif
}

#ifdef ADD_PAN
/*
 * color is broken i beleive it works for Pallete base only
 */
#endif


/*----------------------------------------------------------------*
 |                           GetXorGC                             |
 *----------------------------------------------------------------*/
static GC
GetXorGC (Widget w)
{
  GC gc;
  XGCValues values;

  XtVaGetValues (w, XmNforeground, &values.foreground,
                 XmNbackground, &values.background, NULL);

  values.foreground = values.foreground ^ values.background;
  values.function = GXxor;
  values.line_style = LineOnOffDash;

  gc = XtGetGC (w,
                GCForeground | GCBackground | GCFunction | GCLineStyle,
                &values);

  return (gc);
}


/*----------------------------------------------------------------*
 |                         GetCanvasGC                            |
 *----------------------------------------------------------------*/
static GC
GetCanvasGC (Widget w)
{
  GC gc;
  XGCValues values;

  XtVaGetValues (w, XmNforeground, &values.foreground,
                 XmNbackground, &values.background, NULL);

  values.foreground = values.background;
  values.function = GXcopy;

  gc = XtGetGC (w, GCForeground | GCBackground | GCFunction, &values);

  return (gc);
}


#ifndef ADD_PAN
/*----------------------------------------------------------------*
 |                       SetWindowColor                           |
 *----------------------------------------------------------------*/
static void
SetWindowColor (PannerInfoRec * pInfo, int i)
{
  /* should not happen unless a bug or type is in code */
  if ((pInfo == (PannerInfoRec *) NULL) || (pInfo->display == NULL))
    return;

#ifdef ADD_COLOR
  if (color_fail == 2)
    XSetForeground (pInfo->display, canvasGC, colorw[i % NUMBER_OF_COLORS]);
  else
#endif
    XSetForeground (pInfo->display, canvasGC, cells[(i + 1) % COLOR_COUNT]);
}
#endif

/*----------------------------------------------------------------*
 |                     TranslateCoordinates                       |
 *----------------------------------------------------------------*/
static void
TranslateCoordinates (PannerInfoRec * pInfo,
                      int x1, int y1, unsigned int width1,
                      unsigned int height1, int *x2, int *y2,
                      unsigned int *width2, unsigned int *height2)
{
  int rootW, rootH;

  /* should not happen unless a bug or type is in code */
  if (!pInfo)
    return;

  rootW = pInfo->screenW;
  rootH = pInfo->screenH;

  *x2 = (float) x1 * (float) pInfo->thumbW / (float) rootW + (float) pInfo->thumbX;
  *y2 = (float) y1 * (float) pInfo->thumbH / (float) rootH + (float) pInfo->thumbY;
  *width2 = (float) width1 * (float) pInfo->thumbW / (float) rootW;
  *height2 = (float) height1 * (float) pInfo->thumbH / (float) rootH;
}


/*----------------------------------------------------------------*
 |                          IgnoreError                           |
 *----------------------------------------------------------------*/
static int
IgnoreError (Display * dsp, XErrorEvent * event)
{
  /*
   * Do Nothing...
   * This is needed since we will may be updating the list of window
   * while one of them goes away.  Ie: the window list received from
   * XQueryTree may not be valid for the entire loop where we get each
   * window's geometry.
   */
  return 0;                     /* make compiler happy */
}



/*======================= TRACKING HANDLERS ======================*/

#ifdef ADD_PAN
static void
panEdge (XEvent * event)
{
  int x, y, rootW, rootH;

  if (tracking)
    return;
  if ((DPY_ACT == NULL))
    return;
  if ((SCR_ACT == NULL))
    return;
  /* if (event->xmotion.type == MotionNotify ) */
  rootW = SCR_ACTW;
  rootH = SCR_ACTH;
#ifdef PAN_DEBUG
  fprintf (stderr, " ev (%d,%d) ? scr (%d,%d)\n",
           event->xmotion.x_root, event->xmotion.y_root, rootW, rootH);
#endif
  x = 0;
  y = 0;
  if (event->xmotion.x_root <= 0)
    x = edge_speed;
  if (event->xmotion.x_root >= rootW - 1)
    x = -edge_speed;
  if (event->xmotion.y_root <= 0)
    y = edge_speed;
  if (event->xmotion.y_root >= rootH - 1)
    y = -edge_speed;
  if (!x && !y)
    return;
  {
    Display *display;
    Time time;
    XPointerMovedEvent *motionEvent = (XPointerMovedEvent *) event;
    Window window;
    Atom MY_PANNER_PROP;
    time = GetTimestamp (DPY_ACT);
    int i;
    float f;
    display = XtDisplay (notebook);
    window = XtWindow (notebook);
    f = edge_slide;
    if ( abs((float) x / f) < 1 && abs((float) y / f) < 1)
      f = 1;
    x = (float) x / f;
    y = (float) y / f;
    i = f;
    for (; i > 0; --i)
    {
      /* XPutBackEvent(DPY_ACT, &event); return; */
      /* MoveScreen (pInfoList[LOCAL], x, y, motionEvent->time); elsewhere */
      PackCARD32 (&data[0], (float) x * edge_mult);
      PackCARD32 (&data[1], (float) y * edge_mult);
      PackCARD32 (&data[2], 0);
      PackCARD32 (&data[3], 0);
      PackCARD32 (&data[4], 0);
      PackCARD32 (&data[5], 0);
      PackCARD32 (&data[6], 0);
      if (x_only)
      {
        MY_PANNER_PROP = XInternAtom (display, "MY_PANNER_PROP", False);
        XChangeProperty (display, window, MY_PANNER_PROP, MY_PANNER_PROP,
                         PAN_FORMAT, PropModeReplace,
                         (unsigned char *) fulldata, PAN_ELEM);
        XConvertSelection (DPY_ACT, pInfoList[DSP].WM, pInfoList[DSP].WM_PAN,
                           MY_PANNER_PROP, window, time);
        /* motionEvent->time */
      }
      else
      {
        if (!XmeNamedSink (pInfoList[DSP].utmShell, pInfoList[DSP].WM,
                           XmCOPY, (XtPointer) fulldata, time))
          printf ("Error - UTM Transfer failed.\n");
      }
      if ((++edge_cnt % (int) edge_update) == 0)
        UpdatePannerView (pInfoList, DSP);
    }
  }
}
#endif


/*----------------------------------------------------------------*
 |                         StartTracking                          |
 *----------------------------------------------------------------*/
static void
StartTracking (Widget w,
               XtPointer clientData, XEvent * event, Boolean * dispatch)
{
  PannerInfoRec *pInfo = (PannerInfoRec *) clientData;
  XPointerMovedEvent *motionEvent = (XPointerMovedEvent *) event;

  if (tracking)
    return;
#ifdef ADD_PAN
  if (event->xbutton.button == Button1 || event->xbutton.button == Button3)
  {
    if (event->xbutton.button == Button3)
    {
      button3 = True;
      tracking = True;
      track_count = 0;
      pInfo->oldEventX = event->xbutton.x;
      pInfo->oldEventY = event->xbutton.y;
    }
    /* FIXME there is no firm b3 up down to go by , tracking is spurious
     *       and last MoveDesktop is done with no buttons (b1,3 ignored) always
     */
    if (event->xbutton.button == Button1)
    {
      button3 = False;
      tracking = True;
      track_count = 0;
    }
#else
  if ((pinnedState == VERIFIED) && (event->xbutton.button == Button1))
  {
#endif
    pInfo->lastEventX = event->xbutton.x;
    pInfo->lastEventY = event->xbutton.y;

    /*
     * if on the canvas, then center the thumb over the pointer.
     */
    if (!button3)
      if ((event->xbutton.x < pInfo->thumbX) ||
          (event->xbutton.y < pInfo->thumbY) ||
          (event->xbutton.x > pInfo->thumbX + (int) pInfo->thumbW) ||
          (event->xbutton.y > pInfo->thumbY + (int) pInfo->thumbH))
      {
        centering = True;
        MoveScreen (pInfo,
                    (float) event->xbutton.x - (float) pInfo->thumbW / (float) 2,
                    (float) event->xbutton.y - (float) pInfo->thumbH / (float) 2,
                    motionEvent->time);
      }

    XtAddEventHandler (w, ButtonReleaseMask, False, StopTracking, clientData);
#ifdef ADD_PAN
    if (button3)
    {
      XtAddEventHandler (w, Button3MotionMask, False, DoTracking, clientData);
    }
    else
    {
      XtAddEventHandler (w, Button1MotionMask, False, DoTracking, clientData);
    }
#endif
  }
#ifndef ADD_PAN
  else if (pinnedState != VERIFIED)
    CheckPinnedState ();
#endif
}


/*----------------------------------------------------------------*
 |                          DoTracking                            |
 *----------------------------------------------------------------*/
static void
DoTracking (Widget w,
            XtPointer clientData, XEvent * event, Boolean * dispatch)
{
  PannerInfoRec *pInfo = (PannerInfoRec *) clientData;
  XPointerMovedEvent *motionEvent = (XPointerMovedEvent *) event;

  /* should not happen unless a bug or type is in code */
  if ((pInfo == (PannerInfoRec *) NULL))
    return;

#ifdef ADD_PAN
  if (!tracking)
    return;
  ++track_count;
  if( ! button3 )
    if( (track_count % (int) pan_mult) != 0 )
      return;
#endif

  MoveScreen (pInfo,
              pInfo->thumbX + event->xbutton.x - pInfo->lastEventX,
              pInfo->thumbY + event->xbutton.y - pInfo->lastEventY,
              motionEvent->time);

  pInfo->lastEventX = event->xbutton.x;
  pInfo->lastEventY = event->xbutton.y;
}


/*----------------------------------------------------------------*
 |                         StopTracking                           |
 *----------------------------------------------------------------*/
static void
StopTracking (Widget w,
              XtPointer clientData, XEvent * event, Boolean * dispatch)
{

  tracking = False;

  if (event->xbutton.button == Button1)
  {
    XtRemoveEventHandler (w, Button1MotionMask, False, DoTracking,
                          clientData);
  }
#ifdef ADD_PAN
  /* note: spurious does NOT mean button3 is up or drag is done */
  if (event->xbutton.button == Button3)
  {
    XtRemoveEventHandler (w, Button3MotionMask, False, DoTracking,
                          clientData);
  }
  /* this thumbX update controls if view centers after drag finishes */
  pInfoList[DSP].thumbX =
    (float) pInfoList[DSP].canvasW / (float) 2 - (float) pInfoList[DSP].thumbW / (float) 2;
  pInfoList[DSP].thumbY =
    (float) pInfoList[DSP].canvasH / (float) 2 - (float) pInfoList[DSP].thumbH / (float) 2;
#endif
  XtRemoveEventHandler (w, ButtonReleaseMask, False, StopTracking,
                        clientData);

  UpdatePannerView (pInfoList, DSP);
}



/*----------------------------------------------------------------*
 |                          MoveScreen                            |
 *----------------------------------------------------------------*/
static void
MoveScreen (PannerInfoRec * pInfo, int newX, int newY, Time time)
{
#ifdef ADD_PAN
  static int busy = False;
  /* static int offsetX=0, offsetY=0; */
  int oldthumbX, oldthumbY;
#endif
  Window win;
#ifdef TRY_ALLOC_ON_MULTISCREENS
  static XtPointer msg;
  static XtPointer fulldata;
#endif
  XWindowAttributes attr;
  int dx, dy, panDx, panDy, rootW, rootH, config, x, y;
  unsigned long size;

#ifdef ADD_PAN
  if (busy)
  {
#if defined(ADD_PAN) && defined(PAN_DEBUG)
    fprintf (stderr, "busy\n");
    fflush (stderr);
#endif
    return;
  }
  busy = True;
#endif

  /* should not happen unless a bug or type is in code */
  if ((pInfo == (PannerInfoRec *) NULL) || (pInfo->display == NULL))
    return;
  if (button3 && (DPY_LOC == NULL))
    return;

#if defined(ADD_PAN) && defined(PAN_DEBUG)
  fprintf (stderr, "\n--------------\n");
  fprintf (stderr, "button3 ON\n");
  fflush (stderr);
#endif

#ifdef ADD_PAN
  /* this erases old dashed line by XOR drawning in same place */
  if (button3 && track_count != 0)
    DrawThumbSmall (pInfo);
  else
    DrawThumb (pInfo);
  if (button3 && (track_count % (int) b3_upd_freq == 0))
    UpdatePannerView (pInfoList, DSP);

#endif
  if (button3)
  {
    x = pInfo->lastEventX;
    if (x > pInfo->thumbX)
      x += 5;
    if (x > pInfo->thumbX + pInfo->thumbW)
      x += 4;
    if (x < pInfo->thumbX)
      x -= 3;
    x -= pInfo->thumbX;
    y = pInfo->lastEventY;
    if (y > pInfo->thumbY)
      y += 4;
    if (y > pInfo->thumbY + pInfo->thumbH)
      y += 2;
    if (y < pInfo->thumbY)
      y -= 3;
    y -= pInfo->thumbY;
    /* ? 5 3 is for cursor align ? already tried before after dx updates */
  }

  dx = newX - pInfo->thumbX;
  dy = newY - pInfo->thumbY;

  oldthumbX = pInfo->thumbX;
  oldthumbY = pInfo->thumbY;

#ifdef ADD_PAN
  if (!button3)
#endif
  {
    pInfo->thumbX = newX;
    pInfo->thumbY = newY;
  }

#ifdef ADD_PAN

  pInfo->oldEventX = pInfo->lastEventX;
  pInfo->oldEventY = pInfo->lastEventY;

  /* this erases old dashed line by XOR drawning in same place */
  if (button3)
    DrawThumbSmall (pInfo);
  else
    DrawThumb (pInfo);
#endif

  rootW = pInfo->screenW;
  rootH = pInfo->screenH;

  panDx = -((float) dx * (float) rootW / (float) pInfo->thumbW);
  panDy = -((float) dy * (float) rootH / (float) pInfo->thumbH);

#ifdef ADD_PAN
  config = 1;
  if (button3 && track_count > 1)
    config |= 2;
#endif

#if defined(ADD_PAN) && defined(PAN_DEBUG)
  if (button3)
  {
    win = XtWindow (pInfo->canvas);
    XGetWindowAttributes (DPY_LOC, win, &attr);
    fprintf (stderr, "canvasWH(%d,%d)\n", attr.width, attr.height);
  }
  fprintf (stderr, "dxy=(%d,%d)\nnewXY=(%d,%d)\nthumbXY=(%d,%d)\n",
           dx, dy, newX, newY, oldthumbX, oldthumbY);
  fprintf (stderr, "panDxy=(%d,%d)\nthumbWH=(%d,%d)\n",
           panDx, panDy, pInfo->thumbW, pInfo->thumbH);
  fprintf (stderr, "lastEventXY(%d,%d)\n",
           pInfo->lastEventX, pInfo->lastEventY);
  fprintf (stderr, "dragXY=(%d,%d)\n", x, y);
  fflush (stderr);
#endif

  /*
   * Send Pan message to mwm.
   */

#if defined(TRY_ALLOC_ON_MULTISCREENS) || !defined(ADD_PAN)

  /* SKIP THIS SECTION */
  /* SEE NEXT SECTION */

/* 
 * tries to alloc per screen relies on callback to free for X_ONLY
 * but that might not happen (using ADD_PAN: never happens)
 * instead just make memory global and simple
 */
  size = sizeof (CARD32);       /* panDx */
  size += sizeof (CARD32);      /* panDy */
#ifndef ADD_PAN
  size += sizeof (CARD8);       /* config */
#else
  size += sizeof (CARD32);      /* config */
  size += sizeof (CARD32);      /* panDy */
  size += sizeof (CARD32);      /* panDy */
  size += sizeof (CARD32);      /* panDy */
  size += sizeof (CARD32);      /* panDy */
#endif
#if defined(ADD_PAN) && defined(PAN_DEBUG)
  fprintf (stderr, "allocs=%d\n", msg_xallocs);
  fflush (stderr);
#endif
  if (msg_xallocs)
  {
    XtFree ((char *) fulldata);
    fulldata = NULL;
    --msg_xallocs;
  }
  ++msg_xallocs;
  msg = fulldata = (XtPointer) XtMalloc (sizeof (CARD8) * size);
  msg = PackCARD32 (msg, panDx);
  msg = PackCARD32 (msg, panDy);
#ifdef ADD_PAN
  /* mwm expects 1 if panner is calling, |2 if panner says b3 is held */
  config = 1;
  if (button3 && track_count > 1)
    config |= 2;
  msg = PackCARD32 (msg, config);
#else
  msg = PackCARD32 (msg, 1);
#endif
#ifdef ADD_PAN
  if (button3)
  {
    msg = PackCARD32 (msg, x);
    msg = PackCARD32 (msg, y);
    msg = PackCARD32 (msg, pInfo->thumbW);
    msg = PackCARD32 (msg, pInfo->thumbH);
  }
  else
  {
    msg = PackCARD32 (msg, 0);
    msg = PackCARD32 (msg, 0);
    msg = PackCARD32 (msg, 0);
    msg = PackCARD32 (msg, 0);
  }
#endif
#endif /* TRY_ALLOC_ON_MULTISCREENS SKIP */


#if defined(ADD_PAN) && !defined(TRY_ALLOC_ON_MULTISCREENS)

  /* because byte order of hosts likely differ (and mwm uses UnpackCARD32)
   * (pan could be run from rhost -display thishost ) */

  /* a quirk is x_only works with b3 - applying mult makes it worse */
  if (!button3 && x_only && !centering )
  {
    PackCARD32 (&data[0], (float) panDx );
    PackCARD32 (&data[1], (float) panDy );
  }
  else
  {
    PackCARD32 (&data[0], (float) panDx);
    PackCARD32 (&data[1], (float) panDy);
  }
  centering = False;
  /* mwm expects 1 if panner is calling, |2 if panner says b3 is held */
  PackCARD32 (&data[2], config);
  if (button3 )
  {
    PackCARD32 (&data[3], x);
    PackCARD32 (&data[4], y);
    PackCARD32 (&data[5], pInfo->thumbW);
    PackCARD32 (&data[6], pInfo->thumbH);
  }
  else
  {
    PackCARD32 (&data[3], 0);
    PackCARD32 (&data[4], 0);
    PackCARD32 (&data[5], 0);
    PackCARD32 (&data[6], 0);
  }
#endif /* ADD_PAN && !TRY_ALLOC_ON_MULTISCREENS */

#if defined(ADD_PAN) || defined(X_ONLY)
  /* see Note 1 at EOF */
#ifdef ADD_PAN
  if (x_only)
#endif
  {
    Display *display;
    Window window;
    Atom MY_PANNER_PROP;

    display = XtDisplay (notebook); /* notebook just happens to be a global. */
    window = XtWindow (notebook);

    MY_PANNER_PROP = XInternAtom (display, "MY_PANNER_PROP", False);
#ifdef ADD_PAN
    LOCK = False;
#endif
    if (!LOCK)
    {
      LOCK = True;
#ifdef ADD_PAN
      /*
       * problem is even if fulldata[i] exists Mwm wont know that (Card32x7)
       * it'd need a protocall+fallbacks to tell it: + we dont know how many !
       *   PropModeAppend is attractive but not easy.
       */
      XChangeProperty (display, window, MY_PANNER_PROP, MY_PANNER_PROP,
                       PAN_FORMAT, PropModeReplace,
                       (unsigned char *) fulldata, PAN_ELEM);
#else
      XChangeProperty (display, window, MY_PANNER_PROP, MY_PANNER_PROP,
                       PAN_FORMAT, PropModeReplace,
                       (unsigned char *) fulldata[fulldata_rot],
                       size);
                       /* elsewhere PAN_SIZE was used */
#endif
      XConvertSelection (pInfo->display, pInfo->WM, pInfo->WM_PAN,
                         MY_PANNER_PROP, window, time);
    }
  }
  else
#endif /* X_ONLY */
  {
    /* this gets a callback from Xme so append is ignored , taken care of */

    if (!XmeNamedSink (pInfo->utmShell, /* widget with destination callback */
                       pInfo->WM, /* named selection - ie. Window Manager */
                       XmCOPY,  /* operation to perform on the data */
                       (XtPointer) fulldata,  /* pointer to param data, */
                       time)    /*  free n TransferDone proc. */
      )
      printf ("Error - UTM Transfer failed.\n");
  }

#ifdef ADD_PAN
  busy = False;
#endif

}



/*----------------------------------------------------------------*
 |                      PACKING FUNCTIONS                         |
 *----------------------------------------------------------------*/

static XtPointer
PackCARD32 (XtPointer data, CARD32 val)
{
  CARD16 bottom = val & (0xFFFF);
  CARD16 top = val >> 16;

  data = PackCARD16 (data, top);
  data = PackCARD16 (data, bottom);
  return (data);
}


static XtPointer
PackCARD16 (XtPointer data, CARD16 val)
{
  CARD8 bottom = val & (0xFF);
  CARD8 top = val >> 8;

  data = PackCARD8 (data, top);
  data = PackCARD8 (data, bottom);
  return (data);
}


static XtPointer
PackCARD8 (XtPointer data, CARD8 val)
{
  CARD8 *ptr = (CARD8 *) data;

  *ptr = (CARD8) val;
  data = ((char *) data) + 1;
  return (data);
}



/*======================= USER INTERFACE ======================*/

/*----------------------------------------------------------------*
 |                         CreateMenuBar                          |
 *----------------------------------------------------------------*/
static void
CreateMenuBar (Widget parent)
{
  Cardinal n;
  Arg args[10];
  Widget menuBar;
  Widget cascade1, cascade2, cascade3, cascade4;
  Widget menuPane1, menuPane2, menuPane3;
  Widget b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;

  menuBar = XmCreateMenuBar (parent, "menuBar", NULL, 0);

  menuPane1 = XmCreatePulldownMenu (menuBar, "menuPane1", NULL, 0);
  menuPane2 = XmCreatePulldownMenu (menuBar, "menuPane2", NULL, 0);
  menuPane3 = XmCreatePulldownMenu (menuBar, "menuPane3", NULL, 0);

  b1 =
    XtCreateManagedWidget ("b1", xmPushButtonWidgetClass, menuPane1, NULL, 0);

  XtAddCallback (b1, XmNactivateCallback, MenuCB, (XtPointer) MENU_QUIT);

  b2 =
    XtCreateManagedWidget ("b2", xmPushButtonWidgetClass, menuPane2, NULL, 0);

  XtAddCallback (b2, XmNactivateCallback, MenuCB, (XtPointer) MENU_UPDATE);

  b3 =
    XtCreateManagedWidget ("b3", xmPushButtonWidgetClass, menuPane2, NULL, 0);

  XtAddCallback (b3, XmNactivateCallback, MenuCB, (XtPointer) MENU_NEW);

#ifdef ADD_PAN
  b4 =
    XtCreateManagedWidget ("b4", xmPushButtonWidgetClass, menuPane3, NULL, 0);
  XtAddCallback (b4, XmNactivateCallback, MenuCB, (XtPointer) EDGE_START);

  b5 =
    XtCreateManagedWidget ("b5", xmPushButtonWidgetClass, menuPane3, NULL, 0);
  XtAddCallback (b5, XmNactivateCallback, MenuCB, (XtPointer) EDGE_STOP);

  b6 =
    XtCreateManagedWidget ("b6", xmPushButtonWidgetClass, menuPane3, NULL, 0);
  XtAddCallback (b6, XmNactivateCallback, MenuCB, (XtPointer) EDGE_SPEED);

  b7 =
    XtCreateManagedWidget ("b7", xmPushButtonWidgetClass, menuPane3, NULL, 0);
  XtAddCallback (b7, XmNactivateCallback, MenuCB, (XtPointer) EDGE_FREQ);

  b8 =
    XtCreateManagedWidget ("b8", xmPushButtonWidgetClass, menuPane3, NULL, 0);
  XtAddCallback (b8, XmNactivateCallback, MenuCB, (XtPointer) EDGE_SLIDE);

  b9 =
    XtCreateManagedWidget ("b9", xmPushButtonWidgetClass, menuPane3, NULL, 0);
  XtAddCallback (b9, XmNactivateCallback, MenuCB, (XtPointer) PAN_MULT);

  b10 =
    XtCreateManagedWidget ("b10", xmPushButtonWidgetClass, menuPane3, NULL, 0);
  XtAddCallback (b10, XmNactivateCallback, MenuCB, (XtPointer) PAN_MULT);

#endif

  n = 0;
  XtSetArg (args[n], XmNsubMenuId, menuPane1);
  n++;
  cascade1 = XmCreateCascadeButton (menuBar, "cascade1", args, n);
  XtManageChild (cascade1);

  n = 0;
  XtSetArg (args[n], XmNsubMenuId, menuPane2);
  n++;
  cascade2 = XmCreateCascadeButton (menuBar, "cascade2", args, n);
  XtManageChild (cascade2);

  n = 0;
  XtSetArg (args[n], XmNsubMenuId, menuPane3);
  n++;
  cascade3 = XmCreateCascadeButton (menuBar, "cascade3", args, n);
  XtManageChild (cascade3);

  n = 0;
  cascade4 = XmCreateCascadeButton (menuBar, "cascade4", args, n);
  XtAddCallback (cascade4, XmNactivateCallback, MenuCB,
                 (XtPointer) MENU_HELP);
  XtManageChild (cascade4);

  n = 0;
  XtSetArg (args[n], XmNmenuHelpWidget, cascade4);
  n++;
  XtSetValues (menuBar, args, n);

  XtManageChild (menuBar);
}


/*----------------------------------------------------------------*
 |                             MenuCB                             |
 *----------------------------------------------------------------*/
static void
timeoutCB (XtPointer client_data, XtIntervalId * id)
{
  Widget w = (Widget) client_data;
  Window root_return, child_return;
  int root_x_return, root_y_return, win_x_return, win_y_return;
  unsigned int mask_return;
  if (panedge)
    if (XQueryPointer (DPY_ACT, DefaultRootWindow (DPY_ACT), &root_return,
                       &child_return, &root_x_return, &root_y_return,
                       &win_x_return, &win_y_return, &mask_return))
    {
      time_event.xmotion.x_root = root_x_return;
      time_event.xmotion.y_root = root_y_return;
      panEdge (&time_event);
    }
  XtRemoveTimeOut (interval_id);
  if (panedge)
    interval_id = XtAppAddTimeOut (app, edge_freq, timeoutCB, client_data);
}

static void
MenuCB (Widget w, XtPointer clientData, XtPointer callData)
{
  switch ((long) clientData)
  {
  case MENU_UPDATE:
    DoUpdatePanner ();
    break;
  case MENU_NEW:
    DoAddDisplay ();
    break;
  case MENU_QUIT:
    DoQuit ();
    break;
  case MENU_HELP:
    DoHelp ();
    break;
#ifdef ADD_PAN
  case EDGE_SPEED:
  case EDGE_FREQ:
  case EDGE_SLIDE:
  case EDGE_MULT:
  case PAN_MULT:
    which_num = (long) clientData;
    DoNumEdge ();
    break;
  case EDGE_START:
    DoStartEdge (clientData);
    break;
  case EDGE_STOP:
    /* if(panedge)
     *   XSelectInput(DPY_ACT,DefaultRootWindow(DPY_ACT),NoEventMask); */
    DoStopEdge (clientData);
    break;
#endif
  }
}


/*----------------------------------------------------------------*
 |                         DoUpdatePanner                         |
 *----------------------------------------------------------------*/
static void
DoUpdatePanner ()
{
  Window win;
  XWindowAttributes attr;

#ifdef ADD_PAN
  /* jic, reset what is half stateless */
  button3 = False;
  tracking = False;
  track_count = 0;
#endif
  LOCK = False;

  if (DPY_LOC == NULL)
    return;
  SCR_LOCW = WidthOfScreen (SCR_LOC);
  SCR_LOCH = HeightOfScreen (SCR_LOC);
  win = XtWindow (pInfoList[LOCAL].canvas);
  XGetWindowAttributes (DPY_LOC, win, &attr);
  pInfoList[LOCAL].canvasW = attr.width;
  pInfoList[LOCAL].canvasH = attr.height;
  if (DPY_ACT != NULL)
  {
  SCR_ACTW = WidthOfScreen (SCR_ACT);
  SCR_ACTH = HeightOfScreen (SCR_ACT);
  win = XtWindow (pInfoList[DSP].canvas);
  XGetWindowAttributes (DPY_LOC, win, &attr);
  pInfoList[DSP].canvasW = attr.width;
  pInfoList[DSP].canvasH = attr.height;
  }
#ifdef ADD_PAN
  UpdatePannerView (pInfoList, DSP);
#else
  XClearArea (DPY_LOC, XtWindow (pInfoList[DSP].canvas), 0, 0, 0, 0, False);
  DrawWindows (pInfoList);
  DrawThumb (&pInfoList[DSP]);
#endif
}


/*----------------------------------------------------------------*
 |                          DoAddDisplay                          |
 *----------------------------------------------------------------*/
static void
DoAddDisplay ()
{
  static Widget dlog = NULL;
  Arg args[3];
  int n;

Object obj, objp;

  if (dlog == NULL)
  {
    n = 0;
    XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
    n++;
    dlog = XmCreatePromptDialog (pInfoList[LOCAL].shell, "AddDisplayDialog",
                                 args, n);
    XtAddCallback (dlog, XmNokCallback, DoAddDisplayCB, pInfoList);
    XtUnmanageChild (XmSelectionBoxGetChild (dlog, XmDIALOG_HELP_BUTTON));
  }

  XtManageChild (dlog);
}
#if 0
  fprintf(stderr,"panner managechild: wait... \n");fflush(stderr);
  fprintf(stderr,"panner managechild: back \n");fflush(stderr);
#endif
  /*
   * ie, in any app here is were you get 5 sec block/hang if Mwm does
   * not Handle Configure Notify when Xt does XConfigureWindow then
   * sets property XSetWMNormalHints on (almost) final dialog box which
   * needs a WM wrapping
   */

/*----------------------------------------------------------------*
 |                              DoHelp                            |
 *----------------------------------------------------------------*/
static void
DoHelp ()
{
  static Widget dlog = NULL;
  Arg args[3];
  int n;

  if (dlog == NULL)
  {
    dlog = XmCreateInformationDialog (pInfoList[LOCAL].shell, "HelpDialog",
                                      NULL, 0);
    XtUnmanageChild (XmMessageBoxGetChild (dlog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild (dlog, XmDIALOG_CANCEL_BUTTON));
  }

  XtManageChild (dlog);
}


/*----------------------------------------------------------------*
 |                              DoQuit                            |
 *----------------------------------------------------------------*/
static void
DoQuit ()
{
  XSync (DPY_LOC, False);
  XCloseDisplay (DPY_LOC);

  exit (0);
}

#ifdef ADD_PAN

/*----------------------------------------------------------------*
 |                          DoNumEdge                             |
 *----------------------------------------------------------------*/

static void
DoNumEdge ()
{
  static Widget dlog = NULL;
  Arg args[3];
  int n;

  if (dlog == NULL)
  {
    n = 0;
    XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
    n++;
    dlog = XmCreatePromptDialog (pInfoList[LOCAL].shell, "NumberDialog", args, n);
    XtAddCallback (dlog, XmNokCallback, DoNumEdgeCB, pInfoList);
    XtUnmanageChild (XmSelectionBoxGetChild (dlog, XmDIALOG_HELP_BUTTON));
  }
  XtManageChild (dlog);
}
#endif


/*----------------------------------------------------------------*
 |                         GetTimeStamp                           |
 *----------------------------------------------------------------*/
Time
GetTimestamp (Display * dsp)
{
  XEvent event;
  XWindowAttributes attr;
  Atom timeProp = XInternAtom (dsp, "_MOTIF_CURRENT_TIME", False);
  Window rwin = DefaultRootWindow (dsp);

  XGetWindowAttributes (dsp, rwin, &attr);

  if (!(attr.your_event_mask & PropertyChangeMask))
    XSelectInput (dsp, rwin, attr.your_event_mask | PropertyChangeMask);

  XChangeProperty (dsp, rwin, timeProp, timeProp, 8, PropModeAppend, NULL, 0);

  XWindowEvent (dsp, rwin, PropertyChangeMask, &event);

  if (!(attr.your_event_mask & PropertyChangeMask))
    XSelectInput (dsp, rwin, attr.your_event_mask);

  return (event.xproperty.time);
}


/*----------------------------------------------------------------*
 |                      CheckPinnedState                          |
 *----------------------------------------------------------------*/
/* always pinned and WM has no pin supp. */
#ifndef ADD_PAN
static void
CheckPinnedState ()
{
  static int panDx = 0, panDy = -1;
  XtPointer msg, fulldata;
  unsigned long size;
  Window rWin, child;
  int x, y;
  unsigned int width, height, bWidth, depth;
  Time time = GetTimestamp (DPY_LOC);

  if (pinnedState == VERIFIED)
    return;

  panDy = -panDy;

  pinnedState = VERIFYING;

  /* Get position of the top-level shell */
  XGetGeometry (DPY_LOC, XtWindow (pInfoList[LOCAL].shell),
                &rWin, &x, &y, &width, &height, &bWidth, &depth);

  XTranslateCoordinates (DPY_LOC,
                         XtWindow (pInfoList[LOCAL].shell), rWin, x, y,
                         &origX, &origY, &child);

  size = sizeof (CARD32);       /* panDx */
  size += sizeof (CARD32);      /* panDy */
  size += sizeof (CARD8);       /* config */

  msg = fulldata = (XtPointer) XtMalloc (sizeof (CARD8) * size);

  msg = PackCARD32 (msg, panDx);
  msg = PackCARD32 (msg, panDy);
  msg = PackCARD8 (msg, True);

  if (!XmeNamedSink (pInfoList[LOCAL].utmShell,
                     pInfoList[LOCAL].WM, XmCOPY, (XtPointer) fulldata, time))
    printf ("Error - UTM Transfer failed.\n");
}

/*----------------------------------------------------------------*
 |                      ShowPinStateWarning                       |
 *----------------------------------------------------------------*/
static void
ShowPinStateWarning ()
{
  static Widget dlog = NULL;
  Arg args[3];
  int n;

  if (dlog == NULL)
  {
    dlog = XmCreateWarningDialog (pInfoList[LOCAL].shell, "WarningDialog",
                                  NULL, 0);
    XtUnmanageChild (XmMessageBoxGetChild (dlog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild (dlog, XmDIALOG_CANCEL_BUTTON));
  }
  XtManageChild (dlog);
}

#endif /* ! ADD_PAN */


/*----------------------------------------------------------------*
 |                       HandleInitialExpose                      |
 *----------------------------------------------------------------*/
static void
HandleInitialExpose (Widget w,
                     XtPointer clientData, XEvent * event, Boolean * cont)
{
  XtRemoveEventHandler (w, ExposureMask, False, HandleInitialExpose, NULL);
#ifndef ADD_PAN
  CheckPinnedState ();
#endif
}

/*----------------------------------------------------------------*
 |                      DoStart/StopEdge                          |
 *----------------------------------------------------------------*/

#ifdef ADD_PAN
static void
DoStartEdge (XtPointer clientData)
{
  if (!panedge) /* && DPY_ACT != NULL */
  {
    panedge = True;
    interval_id = XtAppAddTimeOut (app, edge_freq, timeoutCB, clientData);
    /* XSelectInput(DPY_ACT,DefaultRootWindow(DPY_ACT),PointerMotionMask); */
  }
}

static void
DoStopEdge (XtPointer clientData)
{
  /* if(panedge)
   *   XSelectInput(DPY_ACT,DefaultRootWindow(DPY_ACT),NoEventMask); */
  panedge = False;
}
#endif

#ifdef ADD_PAN
  /* NOTES */
  /* XtFree never called for X_ONLY
   * i'd prefer "trash cards" to memory climbs - never just assume someone
   * is calling back and its ok to free memory 1-1 that way
   *   1) store on server 1-at-a-time in clobber mode per host per client
   *   4) oops Xme says static is ok.  do that.
   */
  /* this is only free of msg = fulldata, location_data
   * i dont at all like the idea of hoping for callback to free
   * since that means crashing X or PC if anything is intermittent
   * also: XmTransferValue says this callback should aid in data not free it
   * callback NULL (ie, esp for XtFree) remember callback (message)
   * -->
   * could be lost or even sent 2x X is client server client should be
   * autonomous from server state (changes) or goofs as much as possible
   */
/*
 * ADD_PAN ? gets _MOTIF_WM_PAN_POSITION but doesnt sent any back out
 *   extra complexity of the either waiting for or tangling with is unnecessary
 *   a "real deal" includes timestamping, real order, lost msg handling,
 *   to do that right.  a better referee to be simple: ask server
 */

    /*
     * Note 1 - to really make this work across multiple displays,
     * the window argument must reside on the same display as WM_Si.
     */
    /*
     * Use a lock to make sure the property was read by Mwm.
     * When the pan-property is updated, it's safe to make another
     * conversion.
     */
    /* see also HandlePropertyChange */
    /* for TRY_ALLOC_ON_MULTISCREENS fulldata is XtFreed
     * in HandlePropertyChange (which is NOT guaranteed)
     */
#endif

