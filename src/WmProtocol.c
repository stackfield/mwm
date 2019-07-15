
#include <localdef.h>

/* 
 * @OPENGROUP_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1989, 1990, 1991, 1992, 1993 Open Software Foundation, Inc. 
 * Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 * ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
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
*/ 
/* 
 * Motif Release 1.2.3
*/ 


#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$TOG: WmProtocol.c /main/8 1997/06/18 17:31:34 samborn $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmICCC.h"

/*
 * include extern functions
 */

#include "WmError.h"
#include "WmFunction.h"
#include "WmKeyFocus.h"
#include "WmMenu.h"
#include "WmWinInfo.h"
#ifndef NO_WMQUERY 
#include "WmEvent.h"
#endif /* NO_WMQUERY */
#ifdef PANELIST
#ifdef USE_DT
#include "WmPanelP.h"
#endif
#endif /* PANELIST */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
# include "WmCmd.h"
# include "WmDebug.h"
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

#include <Xm/TransferP.h>

/*
 * Function Declarations:
 */

#include "WmProtocol.h"
#ifndef NO_WMQUERY
static Boolean wmq_convert (Widget w, Atom *pSelection, Atom *pTarget, 
    Atom *pType_return, XtPointer *pValue_return, unsigned long *pLength_return,
    int *pFormat_return);
static Boolean wmq_convert_all_clients (Widget w, int screen,
    Atom *pType_return, XtPointer *pValue_return, unsigned long *pLength_return,
    int *pFormat_return);
static void wmq_list_subtree (ClientData *pCD);
static void wmq_add_xid (XID win);
static void wmq_done (Widget w, Atom *pSelection, Atom *pTarget);
static void wmq_lose (Widget w, Atom *pSelection);
static void wmq_bump_xids(void);
#endif /* NO_WMQUERY */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
static void    OwnWMSelections      (Time timestamp);
static Boolean WMiConvert           (Widget, Atom, Atom,
				     XtPointer, unsigned long, int, Atom *,
				     XtPointer *, unsigned long *, int *);
static void    WMiConvertCB         (Widget, XtPointer, XtPointer);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

void SetPanPosition (int, int);

/*
 * Global Variables:
 */
#ifndef NO_WMQUERY
Atom *xa_WM_QUERY = NULL;
Atom xa_WM_POINTER_WINDOW;
Atom xa_WM_CLIENT_WINDOW;
Atom xa_WM_ALL_CLIENTS;
XID *pXids = NULL;
int numXids = -1;
int curXids = 0;
#endif /* NO_WMQUERY */



/*************************************<->*************************************
 *
 *  SetupWmICCC ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the window manager handling of the inter-client
 *  communications conventions.
 *
 *
 *  Outputs:
 *  -------
 *  (wmGD) = Atoms id's are setup.
 *
 *************************************<->***********************************/

void SetupWmICCC (void)
{
    enum { 
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
      	   XA_TARGETS, XA_MULTIPLE, XA_TIMESTAMP, 
#endif
	   XA_WM_STATE, XA_WM_PROTOCOLS, XA_WM_CHANGE_STATE,
	   XA_WM_SAVE_YOURSELF, XA_WM_DELETE_WINDOW,
	   XA_WM_COLORMAP_WINDOWS, XA_WM_TAKE_FOCUS, XA_MWM_HINTS,
	   XA_MWM_MENU, XA_MWM_MESSAGES, XA_MOTIF_WM_OFFSET,
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL) || !defined(NO_WMQUERY))
	   XA_MOTIF_WM_CLIENT_WINDOW, XA_MOTIF_WM_POINTER_WINDOW,
	   XA_MOTIF_WM_ALL_CLIENTS,
#endif
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	   XA_MOTIF_WM_DEFINE_COMMAND, XA_MOTIF_WM_INCLUDE_COMMAND,
	   XA_MOTIF_WM_REMOVE_COMMAND, XA_MOTIF_WM_ENABLE_COMMAND,
	   XA_MOTIF_WM_DISABLE_COMMAND, XA_MOTIF_WM_RENAME_COMMAND,
	   XA_MOTIF_WM_INVOKE_COMMAND, XA_MOTIF_WM_REQUEST_COMMAND,
	   XA_MOTIF_WM_WINDOW_FLAGS,
#if defined(ADD_PAN)
	   XA_MOTIF_WM_PAN, XA_MOTIF_WM_GOTO,
#endif
	   XA_MOTIF_WM_AUTOMATION, 
#endif
	   XA_COMPOUND_TEXT,
	   NUM_ATOMS };

    static char *atom_names[] = {
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
      	   _XA_TARGETS, _XA_MULTIPLE, _XA_TIMESTAMP, 
#endif
	   _XA_WM_STATE, _XA_WM_PROTOCOLS, _XA_WM_CHANGE_STATE,
	   _XA_WM_SAVE_YOURSELF, _XA_WM_DELETE_WINDOW,
	   _XA_WM_COLORMAP_WINDOWS, _XA_WM_TAKE_FOCUS, _XA_MWM_HINTS,
	   _XA_MWM_MENU, _XA_MWM_MESSAGES, _XA_MOTIF_WM_OFFSET,
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL) || !defined(NO_WMQUERY))
# ifdef _XA_MOTIF_WM_CLIENT_WINDOW
	   _XA_MOTIF_WM_CLIENT_WINDOW, _XA_MOTIF_WM_POINTER_WINDOW,
	   _XA_MOTIF_WM_ALL_CLIENTS, 
# else
	   "_MOTIF_WM_CLIENT_WINDOW", "_MOTIF_WM_POINTER_WINDOW",
	   "_MOTIF_WM_ALL_CLIENTS"
# endif
#endif
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	   _XA_MOTIF_WM_DEFINE_COMMAND, _XA_MOTIF_WM_INCLUDE_COMMAND,
	   _XA_MOTIF_WM_REMOVE_COMMAND, _XA_MOTIF_WM_ENABLE_COMMAND,
	   _XA_MOTIF_WM_DISABLE_COMMAND, _XA_MOTIF_WM_RENAME_COMMAND,
	   _XA_MOTIF_WM_INVOKE_COMMAND, _XA_MOTIF_WM_REQUEST_COMMAND,
	   _XA_MOTIF_WM_WINDOW_FLAGS,
#if defined(ADD_PAN)
	   _XA_MOTIF_WM_PAN, _XA_MOTIF_WM_GOTO,
#endif
	   _XA_MOTIF_WM_AUTOMATION, 
#endif
	   "COMPOUND_TEXT"
    };

    XIconSize sizeList;
    int scr;
    Atom atoms[XtNumber(atom_names)];

    /*
     * Make atoms that are required by the ICCC and mwm.  The atom for
     * _MOTIF_WM_INFO is intern'ed in ProcessMotifWmInfo.
     */
    XInternAtoms(DISPLAY, atom_names, XtNumber(atom_names), False, atoms);


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    wmGD.xa_TARGETS			= atoms[XA_TARGETS];

    wmGD.xa_MULTIPLE			= atoms[XA_MULTIPLE];
    wmGD.xa_TIMESTAMP			= atoms[XA_TIMESTAMP];
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

    wmGD.xa_WM_STATE			= atoms[XA_WM_STATE];
    wmGD.xa_WM_PROTOCOLS		= atoms[XA_WM_PROTOCOLS];
    wmGD.xa_WM_CHANGE_STATE		= atoms[XA_WM_CHANGE_STATE];
    wmGD.xa_WM_SAVE_YOURSELF		= atoms[XA_WM_SAVE_YOURSELF];
    wmGD.xa_WM_DELETE_WINDOW		= atoms[XA_WM_DELETE_WINDOW];
    wmGD.xa_WM_COLORMAP_WINDOWS		= atoms[XA_WM_COLORMAP_WINDOWS];
    wmGD.xa_WM_TAKE_FOCUS		= atoms[XA_WM_TAKE_FOCUS];
    wmGD.xa_MWM_HINTS			= atoms[XA_MWM_HINTS];
    wmGD.xa_MWM_MENU			= atoms[XA_MWM_MENU];
    wmGD.xa_MWM_MESSAGES		= atoms[XA_MWM_MESSAGES];
    wmGD.xa_MWM_OFFSET			= atoms[XA_MOTIF_WM_OFFSET];

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    /* wm query targets */
    wmGD._MOTIF_WM_CLIENT_WINDOW  = atoms[XA_MOTIF_WM_CLIENT_WINDOW];
    wmGD._MOTIF_WM_POINTER_WINDOW = atoms[XA_MOTIF_WM_POINTER_WINDOW];
    wmGD._MOTIF_WM_ALL_CLIENTS	  = atoms[XA_MOTIF_WM_ALL_CLIENTS];

    /* intern atoms for Client-Commmand Interface protocol. */
    wmGD._MOTIF_WM_DEFINE_COMMAND = atoms[XA_MOTIF_WM_DEFINE_COMMAND];
    wmGD._MOTIF_WM_INCLUDE_COMMAND= atoms[XA_MOTIF_WM_INCLUDE_COMMAND];
    wmGD._MOTIF_WM_REMOVE_COMMAND = atoms[XA_MOTIF_WM_REMOVE_COMMAND];
    wmGD._MOTIF_WM_ENABLE_COMMAND = atoms[XA_MOTIF_WM_ENABLE_COMMAND];
    wmGD._MOTIF_WM_DISABLE_COMMAND= atoms[XA_MOTIF_WM_DISABLE_COMMAND];
    wmGD._MOTIF_WM_RENAME_COMMAND = atoms[XA_MOTIF_WM_RENAME_COMMAND];
    wmGD._MOTIF_WM_INVOKE_COMMAND = atoms[XA_MOTIF_WM_INVOKE_COMMAND];
    wmGD._MOTIF_WM_REQUEST_COMMAND= atoms[XA_MOTIF_WM_REQUEST_COMMAND];
    wmGD._MOTIF_WM_WINDOW_FLAGS	  = atoms[XA_MOTIF_WM_WINDOW_FLAGS];

#if defined(ADD_PAN)
    wmGD._MOTIF_WM_PAN    = atoms[XA_MOTIF_WM_PAN];
    wmGD._MOTIF_WM_GOTO	  = atoms[XA_MOTIF_WM_GOTO];
#endif

    wmGD._MOTIF_WM_AUTOMATION	  = atoms[XA_MOTIF_WM_AUTOMATION];

#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    /*
     * Assert ownership of the WINDOW_MANAGER selection
     * on each screen that the window manager controls.
     * these use the format WM_Si.
     */
    OwnWMSelections(GetTimestamp());
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

    wmGD.xa_COMPOUND_TEXT = atoms[XA_COMPOUND_TEXT];

#ifndef NO_WMQUERY
    if (!(xa_WM_QUERY = (Atom *) XtMalloc (wmGD.numScreens * (sizeof (Atom)))))
    {
	Warning (((char *)GETMESSAGE(56, 2, "Insufficient memory to XInternAtom _MOTIF_WM_QUERY_nn")));
    }

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	  char wm_query_scr[32];

          sprintf(wm_query_scr, "_MOTIF_WM_QUERY_%d",
                                        wmGD.Screens[scr].screen);
          xa_WM_QUERY[scr] = XInternAtom(DISPLAY, wm_query_scr, False);
        }
        else
        {
          xa_WM_QUERY[scr] = 0;
	}
    }
    xa_WM_CLIENT_WINDOW  = atoms[XA_MOTIF_WM_CLIENT_WINDOW];
    xa_WM_POINTER_WINDOW = atoms[XA_MOTIF_WM_POINTER_WINDOW];
    xa_WM_ALL_CLIENTS    = atoms[XA_MOTIF_WM_ALL_CLIENTS];
#endif /* NO_WMQUERY */


    /*
     * Setup the icon size property on the root window.
     */

    sizeList.width_inc = 1;
    sizeList.height_inc = 1;

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    sizeList.min_width = wmGD.Screens[scr].iconImageMinimum.width;
	    sizeList.min_height = wmGD.Screens[scr].iconImageMinimum.height;
	    sizeList.max_width = wmGD.Screens[scr].iconImageMaximum.width;
	    sizeList.max_height = wmGD.Screens[scr].iconImageMaximum.height;

	    XSetIconSizes (DISPLAY, wmGD.Screens[scr].rootWindow, 
		&sizeList, 1);
	}
    }

#ifndef NO_WMQUERY
    /*
     * Assert ownership of the WM_QUERY selection
     */
    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    if (!XtOwnSelection(wmGD.topLevelW,
				xa_WM_QUERY[scr],
				GetTimestamp(),
				wmq_convert,
				wmq_lose,
				wmq_done))
	      {
		 Warning (((char *)GETMESSAGE(56, 3, "Failed to own _MOTIF_WM_QUERY_nn selection")));
	      }
	}
    }
#endif /* NO_WMQUERY */


} /* END OF FUNCTION SetupWmICCC */



/*************************************<->*************************************
 *
 *  SendConfigureNotify (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a synthetic ConfigureNotify event when
 *  a client window is reconfigured in certain ways (e.g., the window is
 *  moved without being resized).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (window id and client size data)
 *
 *************************************<->***********************************/

void SendConfigureNotify (ClientData *pCD)
{
    XConfigureEvent notifyEvent;

#if 0
    fprintf(stderr,"WmProtocol SendConfigureNotify\n");
#endif

    /*
     * Send a synthetic ConfigureNotify message:
     */

    notifyEvent.type = ConfigureNotify;
    notifyEvent.display = DISPLAY;
    notifyEvent.event = pCD->client;
    notifyEvent.window = pCD->client;
#ifdef PANELIST
#ifdef USE_EMB_CLI
    if (pCD->pECD)
    {
	int rootX, rootY;
	Window wChild;
     	WmFpEmbeddedClientData *pECD = (WmFpEmbeddedClientData *)pCD->pECD;

	/*
	 * The front panel uses clientX, clientY for position in
	 * front panel. Translate to root coords for client's
	 * information.
	 */

	XTranslateCoordinates (DISPLAY, pECD->winParent,
	    ROOT_FOR_CLIENT(pCD), pCD->clientX, pCD->clientY, 
	    &rootX, &rootY, &wChild);

	notifyEvent.x = rootX;
	notifyEvent.y = rootY;
	notifyEvent.width = pCD->clientWidth;
	notifyEvent.height = pCD->clientHeight;
    }
    else
#endif /* USE_EMB_CLI */
#else /* PANELIST */
#endif /* PANELIST */
    if (pCD->maxConfig)
    {
	notifyEvent.x = pCD->maxX;
	notifyEvent.y = pCD->maxY;
	notifyEvent.width = pCD->maxWidth;
	notifyEvent.height = pCD->maxHeight;
    }
    else
    {
	notifyEvent.x = pCD->clientX;
	notifyEvent.y = pCD->clientY;
	notifyEvent.width = pCD->clientWidth;
	notifyEvent.height = pCD->clientHeight;
    }
    notifyEvent.border_width = 0;
    notifyEvent.above = None;
    notifyEvent.override_redirect = False;

    XSendEvent (DISPLAY, pCD->client, False, StructureNotifyMask,
	(XEvent *)&notifyEvent);


} /* END OF FUNCTION SendConfigureNotify */



/*************************************<->*************************************
 *
 *  SendClientOffsetMessage (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a client message containing the offset
 *  between the window position reported to the user and the actual
 *  window position of the client over the root.
 *
 *  This can be used by clients that map and unmap windows to help them
 *  work with the window manager to place the window in the same location
 *  when remapped. 
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (frame geometry info)
 *
 *************************************<->***********************************/

void SendClientOffsetMessage (ClientData *pCD)
{
    long borderWidth = (long)pCD->xBorderWidth;
    long offsetX = pCD->clientOffset.x;
    long offsetY = pCD->clientOffset.y;
      
    XClientMessageEvent clientMsgEvent;

    clientMsgEvent.type = ClientMessage;
    clientMsgEvent.window = pCD->client;
    clientMsgEvent.message_type = wmGD.xa_MWM_MESSAGES;
    clientMsgEvent.format = 32;
    clientMsgEvent.data.l[0] = wmGD.xa_MWM_OFFSET;

    /*
     * Use window gravity to allow the user to specify the window
     * position on the screen  without having to know the dimensions
     * of the decoration that mwm is adding.
     */
    
    switch (pCD->windowGravity)
    {
      case NorthWestGravity:
      default:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case NorthGravity:
	{
	    clientMsgEvent.data.l[1] = borderWidth;
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case NorthEastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case EastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = borderWidth + (offsetY - offsetX)/2;
	    break;
	}
	
      case SouthEastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case SouthGravity:
	{
	    clientMsgEvent.data.l[1] = borderWidth;
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case SouthWestGravity:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case WestGravity:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = borderWidth + (offsetY - offsetX)/2;
	    break;
	}
	
      case CenterGravity:
	{
	    clientMsgEvent.data.l[2] = (offsetY - offsetX)/2;
	    break;
	}
    }

    XSendEvent (DISPLAY, pCD->client, False, NoEventMask,
	(XEvent *)&clientMsgEvent);


} /* END OF FUNCTION SendClientOffsetMessage */


/*************************************<->*************************************
 *
 *  SendClientMsg (window, type, data0, time, pData, dataLen)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a client message event that to a client
 *  window.  The message may be sent as part of a protocol arranged for by
 *  the client with the WM_PROTOCOLS property.
 *
 *
 *  Inputs:
 *  ------
 *  window = destination window for the client message event
 *
 *  type = client message type
 *
 *  data0 = data0 value in the client message
 *
 *  time = timestamp to be used in the event
 *
 *  pData = pointer to data to be used in the event
 *
 *  dataLen = len of data (in 32 bit units)
 *
 *************************************<->***********************************/

void SendClientMsg (Window window, long type, long data0, Time time, long *pData, int dataLen)
{
    XClientMessageEvent clientMsgEvent;
    int i;


    clientMsgEvent.type = ClientMessage;
    clientMsgEvent.window = window;
    clientMsgEvent.message_type = type;
    clientMsgEvent.format = 32;
    clientMsgEvent.data.l[0] = data0;
    clientMsgEvent.data.l[1] = (long)time;
    if (pData)
    {
	/*
	 * Fill in the rest of the ClientMessage event (that holds up to
	 * 5 words of data).
	 */

        if (dataLen > 3)
        {
	    dataLen = 3;
        }
        for (i = 2; i < (2 + dataLen); i++)
        {
	    clientMsgEvent.data.l[i] = pData[i];
        }
    }
    
    
    XSendEvent (DISPLAY, window, False, NoEventMask,
	(XEvent *)&clientMsgEvent);
    XFlush(DISPLAY);


} /* END OF FUNCTION SendClientMsg */



/*************************************<->*************************************
 *
 *  AddWmTimer (timerType, timerInterval, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets a window manager timer of the specified type.
 *
 *
 *  Inputs:
 *  ------
 *  timerType = type of timer to be set
 *
 *  timerInterval = length of timeout in ms
 *
 *  pCD = pointer to client data associated with the timer
 *
 *  return = True if timer could be set
 *
 *************************************<->***********************************/

Boolean AddWmTimer (unsigned int timerType, unsigned long timerInterval, ClientData *pCD)
{
    WmTimer *pWmTimer;


    if (!(pWmTimer = (WmTimer *)XtMalloc (sizeof (WmTimer))))
    {
	Warning (((char *)GETMESSAGE(56, 1, "Insufficient memory for window manager data")));
	return (False);
    }

    /* !!! handle for XtAppAddTimeOut error !!! */
    pWmTimer->timerId = XtAppAddTimeOut (wmGD.mwmAppContext, 
			    timerInterval, (XtTimerCallbackProc)TimeoutProc, (caddr_t)pCD);
    pWmTimer->timerCD = pCD;
    pWmTimer->timerType = timerType;
    pWmTimer->nextWmTimer = wmGD.wmTimers;
    wmGD.wmTimers = pWmTimer;

    return(True);

} /* END OF FUNCTION AddWmTimer */



/*************************************<->*************************************
 *
 *  DeleteClientWmTimers (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes all window manager timers that are associated with
 *  the specified client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for client whose timers are to be deleted
 *
 *  wmGD = (wmTimers)
 *
 *************************************<->***********************************/

void DeleteClientWmTimers (ClientData *pCD)
{
    WmTimer *pPrevTimer;
    WmTimer *pWmTimer;
    WmTimer *pRemoveTimer;


    pPrevTimer = NULL;
    pWmTimer = wmGD.wmTimers;
    while (pWmTimer)
    {
	if (pWmTimer->timerCD == pCD)
	{
	    if (pPrevTimer)
	    {
		pPrevTimer->nextWmTimer = pWmTimer->nextWmTimer;
	    }
	    else
	    {
		wmGD.wmTimers = pWmTimer->nextWmTimer;
	    }
	    pRemoveTimer = pWmTimer;
	    pWmTimer = pWmTimer->nextWmTimer;
	    XtRemoveTimeOut (pRemoveTimer->timerId);
	    XtFree ((char *)pRemoveTimer);
	}
	else
	{
	    pPrevTimer = pWmTimer;
	    pWmTimer = pWmTimer->nextWmTimer;
	}
    }


} /* END OF FUNCTION DeleteClientWmTimers */



/*************************************<->*************************************
 *
 *  TimeoutProc (client_data, id)
 *
 *
 *  Description:
 *  -----------
 *  This function is an Xtk timeout handler.  It is used to handle various
 *  window manager timers (i.e. WM_SAVE_YOURSELF quit timeout).
 *
 *
 *  Inputs:
 *  ------
 *  client_data = pointer to window manager client data
 *
 *  id = Xtk timer id
 *
 *************************************<->***********************************/

void TimeoutProc (caddr_t client_data, XtIntervalId *id)
{
    WmTimer *pPrevTimer;
    WmTimer *pWmTimer;

    
    /*
     * Find out if the timer still needs to be serviced.
     */

    pPrevTimer = NULL;
    pWmTimer = wmGD.wmTimers;
    while (pWmTimer)
    {
	if (pWmTimer->timerId == *id)
	{
	    break;
	}
	pPrevTimer = pWmTimer;
	pWmTimer = pWmTimer->nextWmTimer;
    }

    if (pWmTimer)
    {
	/*
	 * Do the timer related action.
	 */

	switch (pWmTimer->timerType)
	{
	    case TIMER_QUIT:
	    {
		XKillClient (DISPLAY, pWmTimer->timerCD->client);
		break;
	    }

	    case TIMER_RAISE:
	    {
		Boolean sameScreen;

		if ((wmGD.keyboardFocus == pWmTimer->timerCD) &&
		    (pWmTimer->timerCD->focusPriority == 
			(PSD_FOR_CLIENT(pWmTimer->timerCD))->focusPriority) &&
		    (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) &&
		    (pWmTimer->timerCD == GetClientUnderPointer(&sameScreen)))
		{
		    Do_Raise (pWmTimer->timerCD, (ClientListEntry *)NULL, STACK_NORMAL);
		}
		break;
	    }
	}


	/*
	 * Remove the timer from the wm timer list.
	 */

	if (pPrevTimer)
	{
	    pPrevTimer->nextWmTimer = pWmTimer->nextWmTimer;
	}
	else
	{
	    wmGD.wmTimers = pWmTimer->nextWmTimer;
	}
	XtFree ((char *)pWmTimer);
    }

    /*
     * Free up the timer.
     */

    XtRemoveTimeOut (*id);


} /* END OF FUNCTION TimeoutProc */


#ifndef NO_WMQUERY 

/*************************************<->*************************************
 *
 *  Boolean wmq_convert (w, pSelection, pTarget, pType_return, 
 *	pValue_return, pLength_return, pFormat_return)
 *
 *
 *  Description:
 *  -----------
 *  This function converts WM_QUERY selections
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  pSelection - pointer to selection type (atom)
 *  pTarget - pointer to requested target type (atom)
 *
 *  Outputs:
 *  ------
 *  pType_return - pointer to type of data returned (atom)
 *  pValue_return - pointer to pointer to data returned
 *  pLength_return - ptr to length of data returned
 *  pFormat_return - ptr to format of data returned
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
static Boolean
wmq_convert (
    Widget w,
    Atom *pSelection,
    Atom *pTarget,
    Atom *pType_return,
    XtPointer *pValue_return,
    unsigned long *pLength_return,
    int *pFormat_return
    )
{

    Boolean wm_query_found = False;
    int scr;


    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    if (*pSelection == xa_WM_QUERY[scr])
	    {
		wm_query_found = True;
		break;
	    }
	}
    }

    if (wm_query_found)
    {
	if (*pTarget == xa_WM_POINTER_WINDOW)
	{
	    return (False);
	}
	else if (*pTarget == xa_WM_CLIENT_WINDOW)
	{
	    return (False);
	}
	else if (*pTarget == xa_WM_ALL_CLIENTS)
	{
	    return (wmq_convert_all_clients (w, scr, pType_return,
			pValue_return, pLength_return,
			pFormat_return));
	}
    }

    return (wm_query_found);
} /* END OF FUNCTION wmq_convert */


/*************************************<->*************************************
 *
 *  Boolean wmq_convert_all_clients (w, screen, pType_return, 
 *	pValue_return, pLength_return, pFormat_return)
 *
 *
 *  Description:
 *  -----------
 *  This function converts the WM_QUERY selection target WM_ALL_CLIENTS
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  screen - screen number
 *
 *  Outputs:
 *  ------
 *  pType_return - pointer to type of data returned (atom)
 *  pValue_return - pointer to pointer to data returned
 *  pLength_return - ptr to length of data returned
 *  pFormat_return - ptr to format of data returned
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
static Boolean
wmq_convert_all_clients (
    Widget w,
    int screen,
    Atom *pType_return,
    XtPointer *pValue_return,
    unsigned long *pLength_return,
    int *pFormat_return
    )
{
    WmScreenData *pSD = NULL;
    ClientListEntry *pEntry;
    ClientData *pCD;

    /*
     * Start with empty client list
     */
    curXids = 0;

    /*
     * Get all clients on the specified screen
     */

    if (wmGD.Screens[screen].managed) 
    {
	pSD = &wmGD.Screens[screen];
	  
	/*
	 * Traverse the client list for this screen and
	 * add to the list of window IDs 
	 */
	pEntry = pSD->clientList;
	  
	while (pEntry)
	{
	    /* 
	     * Filter out entries for icons
	     */
	    if (pEntry->type != MINIMIZED_STATE)
	    {
		pCD = pEntry->pCD;
		if (pCD->transientChildren)
		{
		    wmq_list_subtree(pCD->transientChildren);
		}
		wmq_add_xid ((XID) pCD->client);
	    }
	    pEntry = pEntry->nextSibling;
	}
    }

    *pType_return = XA_WINDOW;
    *pValue_return = (XtPointer) pXids;
    *pLength_return = curXids;
    *pFormat_return = 32;
    return (True);

} /* END OF FUNCTION wmq_convert_all_clients */


/*************************************<->*************************************
 *
 *  void wmq_list_subtree (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function adds the windows in a transient subtree to the 
 *  global window list
 *
 *  Inputs:
 *  ------
 *  pCD - client data for "leftmost" child of a subtree
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
static void
wmq_list_subtree (
    ClientData *pCD
    )
{

    /*
     * Do children first
     */
    if (pCD->transientChildren)
    {
	wmq_list_subtree(pCD->transientChildren);
    }

    /*
     * Do me
     */
    wmq_add_xid ((XID) pCD->client);

    /*
     * Do siblings
     */
    if (pCD->transientSiblings)
    {
	wmq_list_subtree(pCD->transientSiblings);
    }
	
} /* END OF FUNCTION wmq_list_subtree */



/*************************************<->*************************************
 *
 *  void wmq_add_xid (win)
 *
 *
 *  Description:
 *  -----------
 *  This function adds an xid to the list
 *
 *  Inputs:
 *  ------
 *  win - xid to add
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
static void
wmq_add_xid (
    XID win
    )
{
    if (curXids >= numXids)
    {
	wmq_bump_xids();
    }

    if (curXids < numXids)
    {
	pXids[curXids++] = win;
    }

} /* END OF FUNCTION wmq_add_xid */



/*************************************<->*************************************
 *
 *  void wmq_lose (w, pSelection)
 *
 *
 *  Description:
 *  -----------
 *  This function is called when we lose the WM_QUERY selection
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  pSelection - pointer to selection type (atom)
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *  This shouldn't happen!
 *
 *************************************<->***********************************/
static void
wmq_lose (
    Widget w,
    Atom *pSelection
    )
{
  Warning (((char *)GETMESSAGE(56, 4, "Lost _MOTIF_WM_QUERY_nn selection")));
} /* END OF FUNCTION wmq_lose */



/*************************************<->*************************************
 *
 *  void wmq_done (w, pSelection, pTarget)
 *
 *
 *  Description:
 *  -----------
 *  This function is called when selection conversion is done.
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  pSelection - pointer to selection type (atom)
 *  pTarget - pointer to requested target type (atom)
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *  This is here to prevent Xt from freeing our buffers.
 *
 *************************************<->***********************************/
static void
wmq_done (
    Widget w,
    Atom *pSelection,
    Atom *pTarget
    )
{
} /* END OF FUNCTION wmq_done */



/*************************************<->*************************************
 *
 *  static void wmq_bump_xids ()
 *
 *
 *  Description:
 *  -----------
 *  This function allocates more xids in our local buffer 
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  pSelection - pointer to selection type (atom)
 *  pTarget - pointer to requested target type (atom)
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *  This is here to prevent Xt from freeing our buffers.
 *
 *************************************<->***********************************/
static void
wmq_bump_xids ( void )
{
    XID *px;

    if (pXids)
    {
	if (!(px = (XID *) 
	  XtRealloc ((char *) pXids, (numXids + 32) * (sizeof (XID)))))
	{
	  Warning (((char *)GETMESSAGE(56, 5, "Insufficient memory to convert _MOTIF_WM_QUERY_nn selection")));
	}
	else
	{
	    pXids = px;
	    numXids += 32;
	}
    }
    else
    {
	if (!(pXids = (XID *) 
	  XtMalloc (32 * (sizeof (XID)))))
	{
	  Warning (((char *)GETMESSAGE(56, 5, "Insufficient memory to convert _MOTIF_WM_QUERY_nn selection")));
	}
	else
	{
	    numXids = 32;
	}
    }
}

#endif /* NO_WMQUERY */



#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  static void OwnWMSelections ()
 *
 *
 *  Description:
 *  -----------
 *  Get the selection ownership for each managed screen.  The selection mwm
 *  will own is WM_Si.
 *
 *  Inputs:
 *  ------
 *  
 *  
 *  
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *  
 *
 *************************************<->***********************************/
static void
OwnWMSelections ( Time timestamp )
{
  int scr;
  
  
  wmGD.xa_WM = (Atom *) XtMalloc (wmGD.numScreens * (sizeof (Atom)));
  
  for (scr = 0; scr < wmGD.numScreens; scr++)
    {
      if (wmGD.Screens[scr].managed)
	{
	  char wm_scr[8];
	  
 	  sprintf(wm_scr, "WM_S%d", DefaultScreen(DISPLAY));
	  wmGD.xa_WM[scr] = XInternAtom (DISPLAY, wm_scr, False);
	  
#ifdef MWM_WSM
	  /*
	   * This registers the callback to be invoked when a request
	   * is made against a WSM Protocol target.  The request
	   * callback is stored by the WSM Protocol code and is
	   * invoked in the convert routine (WMiConvert) below.
	   * See WSMProcessProtoTargets().
	   */

	  WSMRegisterRequestCallback(DISPLAY, scr, HandleWsmConvertRequest,
				     NULL);
#endif

	  /*
	   * Own the selection through UTM.  This sets-up a convert function
	   * that is invoked when a convert request is made on this selection.
	   * The convert function is specified in the drawing area's
	   * XmNconvertCallback resource.
	   */

	  XtAddCallback(wmGD.Screens[scr].utmShell, XmNconvertCallback,
			WMiConvertCB, NULL);

	  if (! XmeNamedSource(wmGD.Screens[scr].utmShell,
			       wmGD.xa_WM[scr], timestamp))
	    {
	      Warning (((char *)GETMESSAGE(56, 6,
					   "Failed to own WM_nn selection")));
	    }
	  else
	    {
	      PRINT("Owning selection %s\n", wm_scr);
	    }
	}
    }
}




/*************************************<->*************************************
 *
 *  Boolean WMiConvert ( )
 *
 *
 *  Description:
 *  -----------
 *  This function converts WM_Si selections using the new param selections
 *
 *************************************<->***********************************/

/*ARGSUSED*/
static Boolean
WMiConvert (
     Widget         w,
     Atom           selection,
     Atom           target,
     XtPointer      input,
     unsigned long  inputLen,
     int            inputFmt,
     Atom          *outputType,
     XtPointer     *output,
     unsigned long *outputLen,
     int           *outputFmt)
{
  int      scr;
  Boolean  found = False;

  
  /* set up some defaults. selection code doesn't like garbage! */
  *outputLen = 0;
  *output    = NULL;
  *outputFmt = 8;

  scr = XScreenNumberOfScreen(XtScreen(w));
  if (!wmGD.Screens[scr].managed)
    {
      Warning (((char *)GETMESSAGE(56, 7,
		"Got convert request from unmanaged screen")));
      found = False;
    }
  
  else {
    if (target == wmGD.xa_TARGETS) {
      Atom *targs       = (Atom *)XtMalloc((unsigned) (28 * sizeof(Atom)));
      int   targetCount = 0;
      
      *output = (XtPointer) targs;
      
      /* required targets */
      *targs++ = wmGD.xa_TARGETS;				targetCount++;
      *targs++ = wmGD.xa_MULTIPLE;				targetCount++;
      *targs++ = wmGD.xa_TIMESTAMP;				targetCount++;

#ifdef MWM_WSM
      /* other targets */
      *targs++ = _WSMReqTypeToTarget(DISPLAY, WSM_CONNECT);	targetCount++;
      *targs++ = _WSMReqTypeToTarget(DISPLAY, WSM_EXTENSIONS);	targetCount++;
      *targs++ = _WSMReqTypeToTarget(DISPLAY, WSM_CONFIG_FMT);	targetCount++;
      *targs++ = _WSMReqTypeToTarget(DISPLAY, WSM_GET_STATE);	targetCount++;
      *targs++ = _WSMReqTypeToTarget(DISPLAY, WSM_SET_STATE);	targetCount++;
      *targs++ = _WSMReqTypeToTarget(DISPLAY, WSM_REG_WINDOW);	targetCount++;
#endif
      
      /* menu command interface support */
      *targs++ = wmGD._MOTIF_WM_DEFINE_COMMAND;			targetCount++;
      *targs++ = wmGD._MOTIF_WM_INCLUDE_COMMAND;		targetCount++;
      *targs++ = wmGD._MOTIF_WM_REMOVE_COMMAND;			targetCount++;
      *targs++ = wmGD._MOTIF_WM_ENABLE_COMMAND;			targetCount++;
      *targs++ = wmGD._MOTIF_WM_DISABLE_COMMAND;		targetCount++;
      *targs++ = wmGD._MOTIF_WM_RENAME_COMMAND;			targetCount++;
      *targs++ = wmGD._MOTIF_WM_INVOKE_COMMAND;			targetCount++;
      *targs++ = wmGD._MOTIF_WM_REQUEST_COMMAND;		targetCount++;
      *targs++ = wmGD._MOTIF_WM_WINDOW_FLAGS;			targetCount++;

#if defined(MWM_WSM) || defined(ADD_PAN)
      /* virtual screen support */
      *targs++ = wmGD._MOTIF_WM_PAN;				targetCount++;
      *targs++ = wmGD._MOTIF_WM_GOTO;				targetCount++;
#endif

      /* automation support */

      *targs++ = wmGD._MOTIF_WM_AUTOMATION;			 targetCount++;
      
      *outputType   = XA_ATOM;
      *outputLen = (targetCount * sizeof(Atom)) >> 2;
      *outputFmt = 32;
      
      found = True;
    }

#if defined(MWM_WSM) || defined(ADD_PAN)

    /* see notes at EOF */
    else if (target == wmGD._MOTIF_WM_PAN)
      {
	int	dx, dy;
#ifndef ADD_PAN
	Boolean config;
#else
	int	config, pannerX, pannerY, pannerWidth, pannerHeight;
	dx=0;dy=0;config=0;pannerX=0;pannerY=0;pannerWidth=0;pannerHeight=0;
#endif
#ifdef ADD_PAN
	if( ACTIVE_PSD->usePan == False ) return;
#endif
	dx     		= (int) UnpackCARD32(&input);
	dy     		= (int) UnpackCARD32(&input);
#ifndef ADD_PAN
	config 		= (Boolean) UnpackCARD8(&input);
#else
	config		= (int) UnpackCARD32(&input);
#endif
#ifdef ADD_PAN
	pannerX  	= (int) UnpackCARD32(&input);
	pannerY  	= (int) UnpackCARD32(&input);
	pannerWidth     = (int) UnpackCARD32(&input);
	pannerHeight	= (int) UnpackCARD32(&input);
#endif
#ifndef ADD_PAN
	PanRoot(dx, dy, config);
#endif
#ifdef ADD_PAN
	PanRoot(dx, dy, config, pannerX, pannerY, pannerWidth, pannerHeight);
#endif
	/*
	 * Update the root property
	 */

	SetPanPosition (ACTIVE_PSD->panDx, ACTIVE_PSD->panDy);

        /* only PanRoot should update "center", not here where there is no test
         * also note the sister WM_GOTO does not update it ? very strange
         */
	
	found = True;
      }
    else if (target == wmGD._MOTIF_WM_GOTO)
      {
	int  x, y, dx, dy;

#ifdef ADD_PAN
	if( ACTIVE_PSD->usePan == False ) return;
#endif
	x = (int) UnpackCARD32(&input);
	y = (int) UnpackCARD32(&input);

#if defined(PAN_DEBUG)
        /* know of nothing that gets here */
        fprintf(stderr," # WMiConvert:_MOTIF_WM_GOTO (%d,%d)\n",dx,dy);
#endif

	dx = ACTIVE_PSD->panDx - x;
	dy = ACTIVE_PSD->panDy - y;

#ifndef ADD_PAN
	PanRoot(dx, dy, 1);
#else
	PanRoot(dx, dy, 1, 0, 0, 0, 0);
#endif
	found = True;
      }
    
    /*
     * Handle the workspace manager protocol targets...
     */
    
#if defined(MWM_WSM)
    else if (WSMIsKnownTarget(w, target))
      {
	/*
	 * Unpack data send in request and invoke CB specified
	 * in WSMRegisterRequestCallback.
	 */
	found = WSMProcessProtoTarget
	                (w, target,
			 input, inputLen, inputFmt,
			 outputType, output, outputLen, outputFmt);
      }
#endif /* MWM_WSM */
#endif /* defined(MWM_WSM) || defined(ADD_PAN)
    
    /*
     *  Handle client-command interface targets.
     */
    
    else if (target == wmGD._MOTIF_WM_DEFINE_COMMAND)
      {
	PRINT("Convert request made for _MOTIF_WM_DEFINE_COMMAND.\n");
	DefineCommand(w, target, (MessageData)input, inputLen, inputFmt);
	PRINT("Returning from _MOTIF_WM_DEFINE_COMMAND.\n");
	
	found = True;
      }
    else if (target == wmGD._MOTIF_WM_INCLUDE_COMMAND)
      {
	PRINT("Convert request made for _MOTIF_WM_INCLUDE_COMMAND.\n");
	IncludeCommand(w, target, (MessageData)input, inputLen, inputFmt);
	PRINT("Returning from _MOTIF_WM_INCLUDE_COMMAND.\n");
	
	found = True;
      }
    else if (target == wmGD._MOTIF_WM_REMOVE_COMMAND)
      {
	PRINT("Convert request made for _MOTIF_WM_REMOVE_COMMAND.\n");
	RemoveCommand(w, target, (MessageData)input, inputLen, inputFmt);
	
	found = True;
      }
    else if (target == wmGD._MOTIF_WM_ENABLE_COMMAND)
      {
	PRINT("Convert request made for _MOTIF_WM_ENABLE_COMMAND.\n");
	EnableCommand(w, target, (MessageData)input, inputLen, inputFmt);
	
	found = True;
      }
    else if (target == wmGD._MOTIF_WM_DISABLE_COMMAND)
      {
	PRINT("Convert request made for _MOTIF_WM_DISABLE_COMMAND.\n");
	DisableCommand(w, target, (MessageData)input, inputLen, inputFmt);
	
	found = True;
      }
    else if (target == wmGD._MOTIF_WM_RENAME_COMMAND)
      {
	PRINT("Convert request made for _MOTIF_WM_RENAME_COMMAND.\n");
	RenameCommand(w, target, (MessageData)input, inputLen, inputFmt);
	
	found = True;
      }
    else if (target == wmGD._MOTIF_WM_INVOKE_COMMAND)
      {
	/* Shouldn't get here! */
      }
    else if (target == wmGD._MOTIF_WM_REQUEST_COMMAND)
      {
      }
    else if (target == wmGD._MOTIF_WM_WINDOW_FLAGS)
      {
      }
    
    else if (target == wmGD._MOTIF_WM_AUTOMATION)
      {
	/*
	 * This function needs to pack the necessary info into the
	 * output data variable to send back to the requesting
	 */

	GetAutomationData(input,outputType,output,outputLen,outputFmt);
	found = True;
      }

    else
    {
      Warning (((char *)GETMESSAGE(56, 8,
		"Conversion request made for unknown target type")));
     }
  }
  
  
  return (found);
}




/*************************************<->*************************************
 *
 *  void WMiConvertCB ( )
 *
 *
 *  Description:
 *  -----------
 *  This function is invoked by UTM to handle the convert request
 *  made by a requesting application.
 *
 *
 *  Comments:
 *  --------
 *  This function is set-up as a callback on a drawing area kept on each
 *  screen.  This is done in WmInitWs.c
 *
 *************************************<->***********************************/

/*ARGSUSED*/
static void
WMiConvertCB (
     Widget    w,
     XtPointer clientData,
     XtPointer callData)
{
  XmConvertCallbackStruct *cnv = (XmConvertCallbackStruct *)callData;
  Atom _MOTIF_LOSE_SELECTION =
    XInternAtom(DISPLAY, "_MOTIF_LOSE_SELECTION", False);
  int scr = XScreenNumberOfScreen(XtScreen(w));
  

  /* Check to make sure we're dealing with the right selection.
   */
  if (cnv->selection != wmGD.xa_WM[scr])
    {
      Warning (((char *)GETMESSAGE(56, 9,
		"Conversion request received for unknown selection")));
      return;
    }

  if (cnv->target == _MOTIF_LOSE_SELECTION)
    {
      /* Done with the conversion - free any data used. */
    }

  /* Handle a conversion request with parameter data.
   */
  else
    {
      WMiConvert (w, cnv->selection, cnv->target,
		  cnv->parm, cnv->parm_length, cnv->parm_format,
		  &(cnv->type), &(cnv->value), &(cnv->length), &(cnv->format));
    }
}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


#ifdef ADD_PAN

/***************************************
 *
 * PanRoot ()
 * -------
 *
 *   Non-Virtual Panning support
 *   This PanRoot assumes data may be a lossy, also does not send out events.
 *   simply move all windows wmw wraps, leave all else alone
 *   it isn't unlike WSM groups and may be compatible.  reason: the top level
 *   is too complex to make factorial more by a 2nd virt fake root just for pan
 *     update from 1st: faster, simpler, includes transients
 *
 **************************************/

/* #define PAN_DEBUG */

#include <math.h>  /* sqrt for dist formula */
/* #include "WmManage.h" */

/* arbitrary limits */
#define PAN_MAX_DELTA		1000000	/* 100k desktop size Xi common ? */
#define PAN_MAX_ABS		10000000
#define PAN_MAX_WINS		100000	/* mwm child+transients count limit */
#define IS_PANNER		(config & 1)
#define STILL_PANNING_B3	(config & 2)
#define SET_STILL_PANNING_B3	config |= 2
#define UNSET_STILL_PANNING_B3	config &= ~2
#define PAN_B3_EXTEND_RADIUS	400
/* else XMoved which only 1/2 works.  seems safe and works */
#define PAN_TRANSIENTS_TOO

static int pan_error=0;

void SetPanPosition (int panDx, int panDy) { ; }

static int PanIgnoreError (Display *dsp, XErrorEvent *event)
{
  pan_error = 1;
  return 0;
}

void PanRoot(int dx, int dy, int config,
  int pannerX, int pannerY, int pannerWidth, int pannerHeight)
{
  static 	Boolean 	busy = False ;
  static 	Boolean 	button3held = False ;
  static 	Window 		* button3Client;
  Display 	* dpy;
  Window	root, parent, win, icon_win, * child;
  XWindowAttributes attribs;
  WmScreenData	* pSD; 
  ClientData	* pcd, ** pcd_arr ;
  ClientListEntry * cle ;
  unsigned int	childCount, i, j, k;
  int x, y, width, height, newx, newy;
  Boolean once,once3, button3,changed;
  int (*oldHandler)();

  typedef struct _savexy { int x; int y; int w; int h; int bw; } SaveAttr;
  SaveAttr * attr_arr;

#if defined(PAN_DEBUG)
  if( busy ) fprintf(stderr," #PanRoot: is BUSY >\n");
  fprintf(stderr,"# PanRoot: (%d,%d) %d (%d,%d) (%d,%d)\n",
    dx,dy,config,pannerX,pannerY,pannerWidth,pannerHeight);
  fflush(stderr);
#endif

  if( busy ) return;
  busy=True;
  changed=False;
  pan_error=0;

/* clamp a few things */

  if( dx==0 && dy==0 ) 		goto busyreturn;
  if( ACTIVE_ROOT == 0 )	goto busyreturn;
  if( DISPLAY == NULL )		goto busyreturn;
  if( ACTIVE_PSD == NULL )	goto busyreturn;

  pSD = ACTIVE_PSD;
  pSD->panning = 0;
  if(pSD->usePan == False) goto busyreturn;
  dpy = DISPLAY;

  if( dx 		>  PAN_MAX_DELTA ) 		goto busyreturn;
  if( dy 		>  PAN_MAX_DELTA ) 		goto busyreturn;
  if( dx 		< -PAN_MAX_DELTA )		goto busyreturn;
  if( dy 		< -PAN_MAX_DELTA )		goto busyreturn;
  /* no panDx check is always (0,0) see note below */

/* button3 is assumed by presence of non-zero values
 * if STILL_PANNING_B3 then we want last window panned to continue
 * (otherwise windows sliding past each other might exchage as cursor drag)
 */
button3 = (pannerX==0 && pannerY==0 && pannerWidth == 0 && pannerHeight == 0)
        ? False : True ;
if( button3 )
{
  if( pannerWidth	<  2 )			goto busyreturn;
  if( pannerHeight	<  2 )			goto busyreturn;
  if( pannerWidth	>  PAN_MAX_ABS )	goto busyreturn;
  if( pannerHeight	>  PAN_MAX_ABS )	goto busyreturn;
  if( pannerX		>  PAN_MAX_ABS )	goto busyreturn;
  if( pannerY		>  PAN_MAX_ABS )	goto busyreturn;
  if( pannerX		< -PAN_MAX_ABS )	goto busyreturn;
  if( pannerY		< -PAN_MAX_ABS )	goto busyreturn;
  button3held = STILL_PANNING_B3 ;
}
else
{
  button3held=False;
}
  if( ! button3held )
    button3Client = NULL;

  cle=pSD->clientList;
  if( cle == NULL ) 				goto busyreturn;

  /* even if there are no windows to move the virtual center updates */
  width=ScreenOfDisplay(dpy, pSD->screen)->width;
  height=ScreenOfDisplay(dpy, pSD->screen)->height;

  child=NULL; pcd_arr=NULL; attr_arr=NULL;
  childCount=0;

  /*
   * We need to install an error handler since the window-tree may
   * become invalid while where still processing the list.
   *   (or XtMalloc fail handling)
   */
  oldHandler = XSetErrorHandler(PanIgnoreError);

  /* find (all) current win to possibly cull list above, a double check */
  if (! XQueryTree(dpy, pSD->rootWindow, &root, &parent, &child, &childCount))
    goto freereturn;
  if( !childCount )
    goto freereturn;

#if 1
#define RADIX_BITS_1( pan_rad_val, pan_rad_bitpos) (pan_rad_val & (1<<pan_rad_bitpos))
/* from sort_int.c, a collection of unsigned int sorts
   this sort is quick in the "mid range" of thousands yet easy to use
*/
void
radix_exchange_sort ( unsigned int * a, int L, int R, int b )
{
        unsigned int t;
        int i, j;
        if ( R>L && b >=0 )
        {
                i = L; j = R;
                while ( j != i )
                {
                while ( RADIX_BITS_1 ( a[i], b ) == 0 && i < j ) ++i;
                while ( RADIX_BITS_1 ( a[j], b ) != 0 && j > i ) --j;
                t = a[i]; a[i] = a[j]; a[j] = t;
                }
                if ( RADIX_BITS_1 ( a[R], b ) == 0 ) j++;
                /* if(j) // without this L,R i,j must be int not unsigned */
                radix_exchange_sort ( a, L, j-1, b-1 );
                radix_exchange_sort ( a, j, R, b-1 );
        }
}
if( childCount >= (unsigned) (((unsigned) 1 << 31)-(unsigned) 2) )
{
  fprintf(stderr,"panner: too many childs %u\n", childCount);
  goto freereturn;
}
radix_exchange_sort (child, 0, childCount-1, sizeof(Window *)*8 - 1);
/* b is, ie, ptr size 32 bits -1 */
#undef RADIX_BITS_1
#endif

/* typical linear binary search */
#define pan_lbsearch(k,rec,a,L,R) { \
  unsigned int l,r;l=L;r=R; \
  while(l<r) { \
    rec=(l+r)>>1; \
    if(k>a[rec]) l=rec+1; \
    else r= rec; } \
  rec=l; if(rec>R||a[r]!=k) rec=-1; }

  pcd_arr = (ClientData **)
    XtMalloc(sizeof(ClientData *) * childCount);
  if( pcd_arr == NULL || pan_error )
  {
    fprintf(stderr,"# PanRoot : wmProtocol.c:PanRoot , out of memory");
    goto freereturn;
  }
  memset(pcd_arr,0,sizeof(ClientData *) * childCount);
  /* for(i=0; i < childCount; ++i) pcd_arr[i] = NULL; */
  attr_arr = (SaveAttr *) XtMalloc(sizeof(SaveAttr) * childCount);
  if( attr_arr == NULL || pan_error )
  {
    fprintf(stderr,"# PanRoot : wmProtocol.c:PanRoot , out of memory");
    goto freereturn;
  }

  /*
   * Find mwm win list and mark pinned names (sticky, not panned)
   * if pcd win is found in child[], can later use mwm move not XMove
   * (if pcd win isn't in child[], skip it since X doesnt know about it)
   */
  /* the cle are a simple list in pSD (no subtree) */
  k=0;
  once3=False;
  for( i=0 ; cle && i < childCount && k++ < PAN_MAX_WINS ; ++i )
  {
    if (once3)
    {
      cle=cle->nextSibling;
      if( cle == pSD->lastClient) break;
      if( cle == NULL) break;
    }
    once3=True;
    pcd=cle->pCD;
    if( pcd == NULL) continue;
    if( pcd->clientFrameWin == 0 ) continue;
    once=False;
    /* if panner minimized is in iconbox, cannot pan to iconbox
       (could open new panner or use keys, is just a policy) */
    if( pcd->clientName)
    if( strcmp(pcd->clientName,"panner") == 0 ||
        strcmp(pcd->clientName,"iconbox") == 0 ||
        strcmp(pcd->clientName,"panel") == 0 )
      once=True;
    /* note: child[j] == cle->pCD->clientFrameWin */
    win = pcd->clientFrameWin;
    pan_lbsearch(win,j,child,0,childCount-1);
    if( j == -1 ) continue ;
#if defined(PAN_DEBUG)
    fprintf(stderr,"%s M %p X %p cle %p\n",pcd->clientName, pcd, win,cle);
#endif
    if( once )
    {
      pcd_arr[j] = (ClientData *) 1;
    }
    else
    {
    /* keep first of duplicate pcd, skip rest (see note 1) */
      if( pcd_arr[j] == NULL )
          pcd_arr[j] = pcd;
    }
    /* see note 1 about how mwm does icon v. window: not well, same pcd */
    win = ICON_FRAME_WIN(pcd);
    if( win )
    {
      /* iconFrameWindow in both app/icon pcd are same, dont want either */
      pan_lbsearch(win,j,child,0,childCount-1);
      if( j != -1 )
          pcd_arr[j] = (ClientData *) 1;
    }
#ifdef PAN_TRANSIENTS_TOO
    /* tranients are kept as a binary tree list */
    /* if win found in child[j] put transient pcd in pcd_arr[j] */
    {
      Boolean once2;
      once2 = False;
      void pan_walk (ClientData *pcdNext)
      {
        ClientData *pcdTmp;
        int j;
        if (++k > PAN_MAX_WINS)
        {
          if ( !once2 )
            fprintf(stderr,"# PanRoot : infinite recusion transient tree\n");
          once2 = True;
          return;
        }
        pcdTmp = pcdNext;
#if defined(PAN_DEBUG)
        fprintf(stderr,"\tT %s M %p X %p\n",
          pcdTmp->clientName, pcdTmp, pcdTmp->clientFrameWin);
#endif
        pan_lbsearch(pcdTmp->clientFrameWin,j,child,0,childCount-1);
        if( j == -1 ) return;
        if( once )
        {
          pcd_arr[j] = (ClientData *) 1;
        }
        else
        {
          if( pcd_arr[j] == NULL )
            pcd_arr[j] = pcdTmp;
        }
        while (pcdTmp->transientChildren)
        {
          pan_walk (pcdTmp->transientChildren);
          pcdTmp = pcdTmp->transientChildren;
        }
        pcdTmp = pcdNext;
        while (pcdTmp->transientSiblings)
        {
          pan_walk (pcdTmp->transientSiblings);
          pcdTmp = pcdTmp->transientSiblings;
        }
      }
      pan_walk (pcd);
    }
#endif
  }
  pcd=NULL;

#undef pan_lbsearch

/* if button 3 (move 1 only) and not continuing, find nearest window to grab */
if( button3 && ! STILL_PANNING_B3 )
{
  double pxmul, pymul, pxpos, pypos, wcenterx, wcentery,
    dist, min_dist;
  button3Client = NULL;
  /* convert panXY to screen coord pxypos */
  pxmul=DisplayWidth(dpy, pSD->screen)/pannerWidth;
  pymul=DisplayHeight(dpy, pSD->screen)/pannerHeight;
  pxpos=(float)pannerX*pxmul;
  pypos=(float)pannerY*pymul;
#if defined(PAN_DEBUG)
  fprintf(stderr,"\tdwh=(%d,%d) pxymul=(%f,%f) pxypos=(%f,%f)\n",
    DisplayWidth(dpy, pSD->screen),DisplayHeight(dpy, pSD->screen),
    pxmul,pymul,pxpos,pypos);
#endif
  min_dist=PAN_MAX_ABS+1;
  dist=0;

  /* find closet window to panner adjusted xy */
  for( i=0; i < childCount; ++i)
  {
    if( pcd_arr[i] == (ClientData *) 1 ) continue;
    if ( ! XGetWindowAttributes (dpy, child[i], &attribs) ) continue;
    if ( attribs.override_redirect == True ) continue ;
    /* if ( attribs.map_state != IsViewable ) continue; excludes icons */
    /* is dist too far to even consider ? */
    if (pxpos + PAN_B3_EXTEND_RADIUS < attribs.x ||
        pxpos - PAN_B3_EXTEND_RADIUS > attribs.x+attribs.width ||
        pypos + PAN_B3_EXTEND_RADIUS < attribs.y ||
        pypos - PAN_B3_EXTEND_RADIUS > attribs.y+attribs.height)
      continue;
    /* which of ones in box is closest ? */
    wcenterx=(float)attribs.x+(float)attribs.width/(float)2;
    wcentery=(float)attribs.y+(float)attribs.height/(float)2;
    dist = sqrt(pow((wcenterx-pxpos),2) + pow((wcentery-pypos),2));
#if defined(PAN_DEBUG)
    fprintf(stderr,"<dist %p = %f> \n",child[i],dist);
#endif
    if( dist < min_dist )
    {
      min_dist = dist;
      button3Client = (Window *) child[i];
    }
  }
  if( button3Client == NULL || min_dist > PAN_MAX_ABS )
    goto freereturn;
  else
    SET_STILL_PANNING_B3;

} /* button3 */

  /* move windows */

/* #define PAN_TIME */
#ifdef PAN_TIME
#include <time.h>
  clock_t ct1,ct2;
  ct1 = clock();
#endif

  for( i=0; i < childCount; ++i)
  {
    if( pcd_arr[i] == (ClientData *) 1 )
      continue;
    if( button3 && STILL_PANNING_B3 && (Window *) button3Client != (Window *) child[i] )
    {
      pcd_arr[i] = (ClientData *) 1 ;
      continue;
    }
    if ( ! XGetWindowAttributes (dpy, child[i], &attribs) )
    {
      pcd_arr[i] = (ClientData *) 1 ;
      continue;
    }
    if ( attribs.override_redirect == True )
    {
      pcd_arr[i] = (ClientData *) 1 ;
      continue;
    }
    /* this -switcheroo may be a panner(1) specific need */
    if( button3 )
    {
      dx= -dx;
      dy= -dy;
    }
    newx=attribs.x+dx;
    newy=attribs.y+dy;

    pcd = pcd_arr[i];

    /* prefer the mwm way or there will be glitches until next manual move */
    if( pcd && ! pSD->panUseX )
    {
      /* see note 2 on why icon moves where window does */
      win = pcd->clientFrameWin;
      icon_win = ICON_FRAME_WIN(pcd);
      if( win )
      {
        /* W not incl border dont try to update. True "is client requested" */
        pSD->panning = 1;
        ProcessNewConfiguration (pcd, newx, newy,
                                 pcd->clientWidth, pcd->clientHeight, True);
        pSD->panning = 0;
        changed=True;
        /* tell the below XSendEvent is already done */
        pcd_arr[i] == (ClientData *) 1;
#if defined(PAN_DEBUG)
        fprintf(stderr,"# Mwm client %d : %p %s\n",i,child[i],pcd->clientName);
#endif
      }
      if( icon_win && ! (pcd->pSD->useIconBox && P_ICON_BOX(pcd)) )
      {
        ICON_X(pcd) = newx;
        ICON_Y(pcd) = newy;
        XMoveWindow(DISPLAY, icon_win, newx, newy);
        if ((ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART) &&
            (wmGD.keyboardFocus == pcd))
        {
          MoveActiveIconText(pcd);
        }
#if defined(PAN_DEBUG)
        fprintf(stderr,"# Mwm icon %d : %p %s\n",i,child[i], pcd->clientName);
#endif
      }
    }
    else
    /* XMove all batch then XSendEvent batch to avoid any extra fuss */
    if ( ! pSD->panUseMwm )
    {
      /* hopefully trimmed but proper ProcessNewConfiguration */
      if( pcd && pSD->panUseX )
      {
        pcd->clientX += dx; /* dont try to calc attribs.x - border, hard */
        pcd->clientY += dy;
        /* pcd->frameInfo.x += dx;
         * pcd->frameInfo.y += dy; */
        x = pcd->clientX - pcd->clientOffset.x;
        y = pcd->clientY - pcd->clientOffset.y;
        XMoveWindow (dpy, pcd->clientFrameWin, x, y);
        changed=True;
        icon_win = ICON_FRAME_WIN(pcd);
        if( icon_win && ! (pcd->pSD->useIconBox && P_ICON_BOX(pcd)) )
        {
          /* not += dx see notes */
          ICON_X(pcd) = x;
          ICON_Y(pcd) = y;
          XMoveWindow (dpy, icon_win, x, y);
          if ((ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART) &&
              (wmGD.keyboardFocus == pcd))
          {
            MoveActiveIconText(pcd);
          }
        }
        SetFrameInfo (pcd);
      }
      else
      {
        attr_arr[i].w=attribs.width;
        attr_arr[i].h=attribs.height;
        attr_arr[i].x=newx;
        attr_arr[i].y=newy;
        attr_arr[i].bw=attribs.border_width;
        XMoveWindow (dpy, child[i], newx, newy);
      }
#if defined(PAN_DEBUG)
      fprintf(stderr,"# XMove client %d : %p\n",i, child[i]);
#endif
    }
  }

  /* send out synthetic notify to apps that they were moved  */
  if ( ! pSD->panUseMwm )
  {
  XConfigureEvent notifyEvent;
  memset(&notifyEvent,0,sizeof(notifyEvent));
  for( i=0; i < childCount; ++i)
  {
    if ( pcd_arr[i] == (ClientData *) 1 )
      continue;
    if( pcd_arr[i] && pSD->panUseX )
    {
      SendConfigureNotify (pcd_arr[i]); /* send to pcd->client */
      continue;
    }
    /* ConfigureNotify event struct XConfigureEvent */
    notifyEvent.type = ConfigureNotify;
    notifyEvent.display = DISPLAY;
    notifyEvent.serial = True;
    notifyEvent.event = child[i];
    notifyEvent.window = child[i];
    notifyEvent.x = attr_arr[i].x;
    notifyEvent.y = attr_arr[i].y;
    notifyEvent.width = attr_arr[i].w;
    notifyEvent.height = attr_arr[i].h;
    notifyEvent.border_width = attr_arr[i].bw;
    notifyEvent.above = None;
    notifyEvent.override_redirect = False;
    XSendEvent (DISPLAY, child[i], False, StructureNotifyMask,
                (XEvent *)&notifyEvent);
  }
  }

  if( changed == True )
  {
    /* we're always on (0,0) so set 0,0.  note neither pan or mwm use panDx */
    pSD->panDx = 0;
    pSD->panDy = 0;
#ifdef NEVER
      /* never seen undrawn frame parts.  maybe notifyEvent or PNC did it
       * this polls X so avoid if possible */
      /* cleanup exposed frame parts */
      PullExposureEvents ();
#endif
#ifdef PAN_TIME
    ct2 = clock();
    fprintf(stderr,"\n%u useX=%d useMwm=%d\n",ct2-ct1,
      pSD->panUseX,pSD->panUseMwm);
#endif
  }

  /* nop; */
freereturn:
  /* nop; */

  XSetErrorHandler(oldHandler);
  if(pcd_arr != NULL)
    XtFree((char *) pcd_arr);
  pcd_arr=NULL;
  if (child != NULL)
    XFree((char*) child);
  child = NULL;
  if (attr_arr != NULL)
    XFree((char*) attr_arr);
  attr_arr = NULL;

  /* nop; */
busyreturn:
  /* nop; */

  busy=False;

}

#if defined(PAN_NOTES)

  /* send out "i panned it" atom XA_WM_PANNED ?
   * yet nother issue is panner only updates after pan change: not after
   * window moves.  and it's (intrusive) if mwm must keep finding panner
   * and giving it messasges, also bad if panner is always jammed waiting for
   * messages that were lost ... time time.  ask server win are where they are
   */
/* note 1
 *
 * if its inside iconbox: is checked later and not moved
 * if icon on desktop may have 2xcle but only one in cle[]
 *   the two cle may have diff .type but that's not helpful
 * overwrite pcd_arr[j] they are the same as far as we can see or use
 *
 * Why: iconbox and icon on desktop two cle: all the same pcd if there are two
 * there may be only one if icon is on desktop, or if never 1st minimized
 *   (two diff cle having same ClientData pointer: all same data)
 * Answer becomes: remove icon address from child[i], dont save cle just pcd
 */
/* note 2
 *
 * Mwm assumes window can't move if iconized: so move the desktop of icons
 * mean no windows got moved.  But worse: mwm uses same pcd for both
 * so if one tries to move the window: mwm moves the icon.  To undo that
 * mwm would have to be edited to use (two pcd) remove the assumption both
 * cannot move or even both seen (the ties between should be very loose).
 *
 * !! note XMove(dpy,icon_win,...) does Not move icon - it's for menu post
 *
 * ie, this would make new policy as icon centric but NOT get "separation"
 *    if ((pcd->clientState == MINIMIZED_STATE) &&
 *        (!(pcd->pSD->useIconBox && P_ICON_BOX(pcd))) && icon_win)
 *    ...
 *      newx=ICON_X(pcd)+dx; newy=ICON_Y(pcd)+dy;
 *      ICON_X(pcd) = newx; ICON_Y(pcd) = newy; ...
 */
#endif /* PAN_NOTES */
#if 0
/* void insertion_sort ( SORT_TYPE *a, unsigned int L, unsigned int R ) */
/* if you had any problems implementing radix bits ... paste this in */
{
        unsigned int i, j, tmp, L, R;
        L = 0;
        R = childCount - 1 ;
        for ( i = L+1; i <= R; ++i )
        {
                tmp = child[i];
                j = i;
                while ( child[j - 1] > tmp )
                {
                        child[j] = child[j - 1];
                        --j;
                        if ( j==L ) break ;
                }
                child[j] = tmp;
        }
}
#endif
#endif /* ADD_PAN */

