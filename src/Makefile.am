bin_PROGRAMS = mwm

mwm_SOURCES = WmCDInfo.c WmCDecor.c WmCEvent.c WmCPlace.c WmCmd.c WmColormap.c WmError.c WmEvent.c WmFeedback.c \
              WmFunction.c WmGraphics.c WmIDecor.c WmIPlace.c WmIconBox.c WmImage.c WmInitWs.c WmKeyFocus.c \
              WmMain.c WmManage.c WmMenu.c WmProperty.c WmProtocol.c WmResCvt.c WmResParse.c WmResource.c WmSignal.c \
              WmWinConf.c WmWinInfo.c WmWinList.c WmWinState.c WmWsm.c WmXSMP.c version.c \
              WmWsmLib/debug.c WmWsmLib/disp.c WmWsmLib/free.c WmWsmLib/pack.c \
              WmWsmLib/recv.c WmWsmLib/send.c WmWsmLib/util.c WmWsmLib/utm_send.c \
	      WmBitmap.h WmCDInfo.h WmCDecor.h WmCEvent.h WmCPlace.h WmCmd.h WmColormap.h \
              WmDebug.h WmError.h WmEvent.h WmFeedback.h WmFunction.h WmGlobal.h WmGraphics.h WmIBitmap.h \
              WmICCC.h WmIDecor.h WmIPlace.h WmIconBox.h WmImage.h WmInitWs.h WmKeyFocus.h WmManage.h \
              WmMenu.h WmOL.h WmProperty.h WmProtocol.h WmResCvt.h WmResNames.h WmResParse.h WmResource.h \
              WmSignal.h WmWinConf.h WmWinInfo.h WmWinList.h WmWinState.h WmWsm.h WmXSMP.h \
	      WmWsmLib/utm_send.h  WmWsmLib/wsm_funcs.h WmWsmLib/wsm_proto.h

mwm_CFLAGS  = -DLARGECURSORS -DR2_COMPAT -DUNMAP_ON_RESTART \
               -DCDE_INSTALLATION_TOP=\"@CDE_INSTALLATION_TOP@\" \
               -DCDE_CONFIGURATION_TOP=\"@CDE_CONFIGURATION_TOP@\" \
               -DBITMAPDIR=\"@INCDIR@/bitmaps\" \
               -DMWMRCDIR=\"$(datadir)/mwm\" \
               -DNO_OL_COMPAT -I../include

rcdir = $(datadir)/mwm
dist_rc_DATA = system.mwmrc
