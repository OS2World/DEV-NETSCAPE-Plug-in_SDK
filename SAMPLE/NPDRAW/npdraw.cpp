/***************************************************************************
 *
 * File name   :  npdraw.cpp
 *
 *  Copyright (C) 1996 IBM Corporation
 *
 *      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is
 *      sample code created by IBM Corporation. This sample code is not
 *      part of any standard or IBM product and is provided to you solely
 *      for  the purpose of assisting you in the development of your
 *      applications.  The code is provided "AS IS", without
 *      warranty of any kind.  IBM shall not be liable for any damages
 *      arising out of your use of the sample code, even if they have been
 *      advised of the possibility of such damages.                            
 *
 ***************************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSPROCESS
#define INCL_GPI
#include <os2.h>

#include <string.h>
#include <stdlib.h>

#include <IString.hpp>
#include <strstrea.h>

#ifndef _NPAPI_H_
#include "npapi.h"
#endif

#include "draw.h"

//
// Instance state information about the plugin.
//
// *Developers*: Use this struct to hold per-instance
//               information that you'll need in the
//               various functions in this file.
//

typedef struct _PluginInstance PluginInstance;
typedef struct _PluginInstance
{
    NPWindow*       fWindow;
    HWND            hWnd;
    uint16          fMode;
    HPS             hps;
    NPStream*       Stream;
    ULONG           SegmentID;
    PFNWP           lpfnOldWndProc;
    NPSavedData*    pSavedInstanceData;
    PluginInstance* pNext;
} PluginInstance;

MRESULT APIENTRY
SubClassFunc(HWND hWnd,ULONG Message,MPARAM wParam, MPARAM lParam);

void Draw(PluginInstance *This, HPS hps, POINTL *endPoint, BOOL fPrinting);
void parseStream(PluginInstance *This, istrstream *str, ULONG len);

HMODULE DLLInstance;

extern "C" {
 #if defined ( __cplusplus )
   void __ctordtorInit(void);
   void __ctordtorTerm(void);
 #endif
}

/* _CRT_init is the C run-time environment initialization function.         */
/* It will return 0 to indicate success and -1 to indicate failure.         */
extern "C"
int _CRT_init(void);

#ifdef   STATIC_LINK
/* _CRT_term is the C run-time environment termination function.            */
/* It only needs to be called when the C run-time functions are statically  */
/* linked.                                                                  */
extern "C"
void _CRT_term(void);

#else

/* A clean up routine registered with DosExitList must be used if runtime   */
/* calls are required at exit AND the runtime is dynamically linked.  This  */
/* will guarantee that this clean up routine is run before the library DLL  */
/* is terminated.                                                           */

static void _System cleanup(ULONG ulReason);
#endif


/* __ctordtorInit is the C++ run-time environment initialization function.  */
/* It is required to initialize static objects and destructors are setup    */
extern "C"
   void __ctordtorInit(void);


/* __ctordtorInit is the C++ run-time environment initialization function.  */
/* It is required to insure static objects destructors are run              */
/* You must call this before calling _CRT_term()                            */
extern "C"
   void __ctordtorInit(void);

extern "C"
unsigned long _System _DLL_InitTerm(unsigned long hModule, unsigned long
                                    ulFlag)
{
    DLLInstance = (HMODULE) hModule;
    switch (ulFlag)
    {
        case 0:
            if ( _CRT_init() == -1 )
            {
                return(0UL);
            }
#if defined ( __cplusplus )
            __ctordtorInit();
#endif

#ifndef  STATIC_LINK

         /*******************************************************************/
         /* A DosExitList routine must be used to clean up if runtime calls */
         /* are required at exit and the runtime is dynamically linked.     */
         /*******************************************************************/

            DosExitList(0x0000FF00|EXLST_ADD, cleanup);
#endif
            break;
        case 1:

#if defined ( __cplusplus )
            __ctordtorTerm();
#endif

#ifdef  STATIC_LINK
            _CRT_term();
#endif
            break;
    }

    return 1;
}

#ifndef  STATIC_LINK
static void cleanup(ULONG ulReason)
{
   /* do any DLL cleanup here if needed AND if dynamically linked to the */
   /* C Runtime libraries                                                */
   DosExitList(EXLST_EXIT, cleanup);
   return ;
}
#endif

// A plugin instance typically will subclass the plugin's client window, so
// it can get Windows messages, (such as paint, palettechanged, keybd, etc).
// To do work associated with a specific plugin instance the WndProc which
// receives the Windows messages, (named "SubClassFunc" herein), needs access
// to the "This" (PluginInstance*) ptr.

//* OS2 bugbug -- we need to reserve 4 bytes of window words to correct this
//  gorp.....

// When Navigator registers the plugin client's window class, (the class for
// the window passed in NPP_SetWindow()), Navigator does not reserve any
// "extra" windows bytes.  If it had, the plugin could simply have stored its
// "This" (PluginInstance*) ptr in the extra bytes.  But Nav did not, and the
// plugin cannot, so a different technique must be used.  The technique used
// is to keep a linked list of PluginInstance structures, and walk the list
// to find which one is associated with the window handle.  Inefficient,
// grungy, complicates the code, etc.  C'est la vie ...

PluginInstance* g_pHeadInstanceList = 0;

// Associate the hWnd with pInstance by setting the hWnd member of the
// PluginInstance struct.  Also, add the PluginInstance struct to the list
// if necessary
static void AssociateInstance(HWND hWnd, PluginInstance* pInstance)
{
    pInstance->hWnd = hWnd;     // redundant, but usefull to get hwnd from
                                // pinstance later.
    BOOL rc = WinSetWindowULong(hWnd, QWL_USER, (ULONG)pInstance);
}

// Find the PluginInstance associated with this hWnd and return it
static PluginInstance* GetInstance(HWND hWnd)
{
    return (PluginInstance*)WinQueryWindowULong(hWnd, QWL_USER);
}

//----------------------------------------------------------------------------
// NPP_Initialize:
//----------------------------------------------------------------------------
NPError NPP_Initialize(void)
{
    // do your one time initialization here, such as dynamically loading
    // dependant DLLs
    return NPERR_NO_ERROR;
}


//----------------------------------------------------------------------------
// NPP_Shutdown:
//----------------------------------------------------------------------------
void NPP_Shutdown(void)
{
    // do your one time uninitialization here, such as unloading dynamically
    // loaded DLLs
}


//----------------------------------------------------------------------------
// NPP_New:
//----------------------------------------------------------------------------
NPError NP_LOADDS
NPP_New(NPMIMEType pluginType,
                NPP instance,
                uint16 mode,
                int16 argc,
                char* argn[],
                char* argv[],
                NPSavedData* saved)
{
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    instance->pdata = NPN_MemAlloc(sizeof(PluginInstance));
    PluginInstance* This = (PluginInstance*) instance->pdata;

    if (This == NULL)
        return NPERR_OUT_OF_MEMORY_ERROR;
    //
    // *Developers*: Initialize fields of your plugin
    // instance data here.  If the NPSavedData is non-
    // NULL, you can use that data (returned by you from
    // NPP_Destroy to set up the new plugin instance.
    //

    This->fWindow = 0;
    // mode is NP_EMBED, NP_FULL, or NP_BACKGROUND (see npapi.h)
    This->fMode = mode;
    This->hWnd = 0;
    This->hps = NULL;
    This->Stream = 0;
    This->SegmentID = 0;
    This->pSavedInstanceData = saved;
    This->pNext = 0;

    return NPERR_NO_ERROR;
}


//-----------------------------------------------------------------------------
// NPP_Destroy:
//-----------------------------------------------------------------------------
NPError NP_LOADDS
NPP_Destroy(NPP instance, NPSavedData** save)
{
    if (instance == 0   )
        return NPERR_INVALID_INSTANCE_ERROR;

    PluginInstance* This = (PluginInstance*) instance->pdata;

    //
    // *Developers*: If desired, call NP_MemAlloc to create a
    // NPSavedDate structure containing any state information
    // that you want restored if this plugin instance is later
    // recreated.
    //

    if (This != 0   )
    {
        // destroy old PS and create new PS
        if (! This->hps)
            GpiDestroyPS(This->hps);

        // Remove the subclass for the client window
        if(This->hWnd)
        {
            WinSubclassWindow(This->hWnd, This->lpfnOldWndProc);
        }

        // make some saved instance data if necessary
        if(This->pSavedInstanceData == 0   ) {
            // make a struct header for the data
            This->pSavedInstanceData =
                (NPSavedData*)NPN_MemAlloc(sizeof (struct _NPSavedData));

            // fill in the struct
            if(This->pSavedInstanceData != 0   ) {
                This->pSavedInstanceData->len = 0;
                This->pSavedInstanceData->buf = 0   ;

                // replace the def below and references to it with your data
                #define SIDATA "aSavedInstanceDataBlock"

                // the data
                This->pSavedInstanceData->buf = NPN_MemAlloc(sizeof SIDATA);

                if(This->pSavedInstanceData->buf != 0   ) {
                    strcpy((char*)This->pSavedInstanceData->buf, SIDATA);
                    This->pSavedInstanceData->len = sizeof SIDATA;
                }
            }
        }

        // save some instance data
        *save = This->pSavedInstanceData;

        NPN_MemFree(instance->pdata);
        instance->pdata = 0   ;
    }

    return NPERR_NO_ERROR;
}


//----------------------------------------------------------------------------
// NPP_SetWindow:
//----------------------------------------------------------------------------
NPError NP_LOADDS
NPP_SetWindow(NPP instance, NPWindow* window)
{
    if (instance == 0   )
        return NPERR_INVALID_INSTANCE_ERROR;

    PluginInstance* This = (PluginInstance*) instance->pdata;

    //
    // *Developers*: Before setting fWindow to point to the
    // new window, you may wish to compare the new window
    // info to the previous window (if any) to note window
    // size changes, etc.
    //
    if((window->window != 0   ) && (This->hWnd == 0   ))
    {
        This->fWindow = window;
        This->hWnd    = (HWND)This->fWindow->window;

        // subclass the window
        This->lpfnOldWndProc = WinSubclassWindow(This->hWnd, SubClassFunc);
        AssociateInstance(This->hWnd, This);

        /* paint a background for the drawing */
        RECTL rect;
        WinQueryWindowRect(This->hWnd, &rect);
        WinFillRect(This->hps, &rect, CLR_PALEGRAY);

        // create a PS
        if (! This->hps)
        {
            HDC hdc = WinQueryWindowDC(This->hWnd);
            if (! hdc)
                hdc = WinOpenWindowDC(This->hWnd);
            SIZEL siz = { 0, 0 };
            This->hps = GpiCreatePS(WinQueryAnchorBlock(This->hWnd), hdc, &siz,
                                    PU_PELS | GPIT_NORMAL | GPIA_ASSOC);
        }
    }
    else {
        // if window handle changed
        if(This->hWnd != (HWND)window->window) {
            // remember the new window
            This->fWindow = window;

            // Remove the subclass for the old client window
            WinSubclassWindow(This->hWnd, This->lpfnOldWndProc);

            // remember the new window handle
            This->hWnd = (HWND)This->fWindow->window;

            if(This->hWnd != 0   ) {
                // subclass the new one
                This->lpfnOldWndProc = WinSubclassWindow(This->hWnd,
                                                         SubClassFunc);
                AssociateInstance(This->hWnd, This);
            }

            /* paint a background for the drawing */
            RECTL rect;
            WinQueryWindowRect(This->hWnd, &rect);
            WinFillRect(This->hps, &rect, CLR_PALEGRAY);

            // destroy old PS and create new PS
            if (! This->hps)
                GpiDestroyPS(This->hps);
            HDC hdc = WinQueryWindowDC(This->hWnd);
            if (! hdc)
                hdc = WinOpenWindowDC(This->hWnd);
            SIZEL siz = { 0, 0 };
            This->hps = GpiCreatePS(WinQueryAnchorBlock(This->hWnd), hdc, &siz,
                                    PU_TWIPS | GPIT_MICRO | GPIA_ASSOC);
        }
    }

    return NPERR_NO_ERROR;
}


//----------------------------------------------------------------------------
// NPP_NewStream:
//----------------------------------------------------------------------------
NPError NP_LOADDS
NPP_NewStream(NPP instance,
              NPMIMEType type,
              NPStream *stream,
              NPBool seekable,
              uint16 *stype)
{
    if (instance == 0   )
        return NPERR_INVALID_INSTANCE_ERROR;
    PluginInstance* This = (PluginInstance*) instance->pdata;

    // if your plugin must operate file based, you may wish to do this:
    //    *stype = NP_ASFILE;
    // remember, though, that use of NP_ASFILE is strongly discouraged;
    // your plugin should attempt to work with data as it comes in on
    // the stream if at all possible

    // initialize stream and length (in bytes)
    This->Stream = stream;
    This->SegmentID = 0;

    return NPERR_NO_ERROR;
}


//
// *Developers*:
// These next 2 functions are directly relevant in a plug-in which handles the
// data in a streaming manner.  If you want zero bytes because no buffer space
// is YET available, return 0.  As long as the stream has not been written
// to the plugin, Navigator will continue trying to send bytes.  If the plugin
// doesn't want them, just return some large number from NPP_WriteReady(), and
// ignore them in NPP_Write().  For a NP_ASFILE stream, they are still called
// but can safely be ignored using this strategy.
//

int32 STREAMBUFSIZE = TOTAL_WIDTH;  // get a multiple of the data we understand

//----------------------------------------------------------------------------
// NPP_WriteReady:
//----------------------------------------------------------------------------
int32 NP_LOADDS
NPP_WriteReady(NPP instance, NPStream *stream)
{
    if (instance == 0   )
        return NPERR_INVALID_INSTANCE_ERROR;
    PluginInstance* This = (PluginInstance*) instance->pdata;

    return STREAMBUFSIZE;   // Number of bytes ready to accept in NPP_Write()
}


//----------------------------------------------------------------------------
// NPP_Write:
//----------------------------------------------------------------------------
int32 NP_LOADDS
NPP_Write(NPP instance, NPStream *stream,
          int32 offset, int32 len, void *buffer)
{
    if (instance == 0   )
        return NPERR_INVALID_INSTANCE_ERROR;
    PluginInstance* This = (PluginInstance*) instance->pdata;

    // copy into our buffer only as much as we can handle
    int myLen;
    if (len < STREAMBUFSIZE)
        myLen = len;
    else
        myLen = STREAMBUFSIZE;
    char *myBuf = (char *)malloc(myLen);
    memcpy(myBuf, buffer, myLen);

    /* set retain mode */
    GpiSetDrawingMode(This->hps, DM_DRAWANDRETAIN);
    This->SegmentID++;
    GpiOpenSegment(This->hps, This->SegmentID);

    /* setup transform to graphics commands are scaled from standard 100x100 */
    /* to the size of the window */
    /* Would really like to do this once in NewStream but SetWindow has not */
    /* yet been called so we don't know the size of the window. We could avoid */
    /* setting the transform once by setting a flag but it's not important */
    /* in this example because of the DosSleep later */
    RECTL rcl;
    WinQueryWindowRect(This->hWnd, &rcl);
    MATRIXLF mtlf;
    FIXED afxScale[2] = { (rcl.xRight - rcl.xLeft) * 0x10000 / MAX_X,
                          (rcl.yTop - rcl.yBottom) * 0x10000 / MAX_Y };
    POINTL ptlScaleOffset = {0, 0};
    GpiScale(This->hps, &mtlf, TRANSFORM_REPLACE, afxScale, &ptlScaleOffset);
    GpiSetModelTransformMatrix(This->hps, 9L, &mtlf, TRANSFORM_REPLACE);

    /* put buffer into a stream and parse it */
    istrstream str((char *)buffer, myLen);
    parseStream(This, &str, myLen);
    GpiCloseSegment(This->hps);

    /* free data buffer */
    free(myBuf);

    return myLen;    // The number of bytes accepted.  Return a
                     // negative number here if, e.g., there was an error
                     // during plugin operation and you want to abort the
                     // stream
}


//----------------------------------------------------------------------------
// NPP_DestroyStream:
//----------------------------------------------------------------------------
NPError NP_LOADDS
NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
    if (instance == 0   )
        return NPERR_INVALID_INSTANCE_ERROR;
    PluginInstance* This = (PluginInstance*) instance->pdata;
    This->Stream = 0;
    // if (reason == NPRES_DONE)
    This->SegmentID = 0;

    return NPERR_NO_ERROR;
}


//----------------------------------------------------------------------------
// NPP_StreamAsFile:
//----------------------------------------------------------------------------
void NP_LOADDS
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
   if (instance == 0   )
       return;

   PluginInstance* This = (PluginInstance*) instance->pdata;

   // invalidate window to ensure a redraw
   WinInvalidateRect(This->hWnd, 0, TRUE);
}


//----------------------------------------------------------------------------
// NPP_Print:
//----------------------------------------------------------------------------
void NP_LOADDS
NPP_Print(NPP instance, NPPrint* printInfo)
{
    if(printInfo == 0   )   // trap invalid parm
        return;

    if (instance != 0   )
    {
        PluginInstance* This = (PluginInstance*) instance->pdata;

        if (printInfo->mode == NP_FULL)
        {
            //
            // *Developers*: If your plugin would like to take over
            // printing completely when it is in full-screen mode,
            // set printInfo->pluginPrinted to TRUE and print your
            // plugin as you see fit.  If your plugin wants Netscape
            // to handle printing in this case, set printInfo->pluginPrinted
            // to FALSE (the default) and do nothing.  If you do want
            // to handle printing yourself, printOne is true if the
            // print button (as opposed to the print menu) was clicked.
            // On the Macintosh, platformPrint is a THPrint; on Windows,
            // platformPrint is a structure (defined in npapi.h) containing
            // the printer name, port, etc.
            //
            void* platformPrint = printInfo->print.fullPrint.platformPrint;
            NPBool printOne = printInfo->print.fullPrint.printOne;

            printInfo->print.fullPrint.pluginPrinted = FALSE; // Do the default

        }
        else    // If not fullscreen, we must be embedded
        {
            //
            // *Developers*: If your plugin is embedded, or is full-screen
            // but you returned false in pluginPrinted above, NPP_Print
            // will be called with mode == NP_EMBED.  The NPWindow
            // in the printInfo gives the location and dimensions of
            // the embedded plugin on the printed page.  On the Macintosh,
            // platformPrint is the printer port; on Windows, platformPrint
            // is the handle to the printing device context.
            //
            NPWindow* printWindow = &(printInfo->print.embedPrint.window);
            void* platformPrint = printInfo->print.embedPrint.platformPrint;

            /* get Presentation Space and save it */
            HPS hps = (HPS)platformPrint;
            LONG saveID = GpiSavePS(hps);

            /* create GPI various data structures about the drawing area */
            POINTL offWindow = { -(int)printWindow->x, -(int)printWindow->y };
            POINTL endPoint = { (int)printWindow->width, (int)printWindow->height };
            RECTL rect = { (int)printWindow->x,
                           (int)printWindow->y,
                           (int)printWindow->x + (int)printWindow->width,
                           (int)printWindow->y + (int)printWindow->height };

            /* get model transform so origin is 0,0 */
            MATRIXLF matModel;
            GpiQueryModelTransformMatrix(hps, 9L, &matModel);
            GpiTranslate(hps, &matModel, TRANSFORM_ADD, &offWindow);
            GpiSetModelTransformMatrix(hps, 9L, &matModel, TRANSFORM_REPLACE);

            /* set clipping region so we don't accidently draw outside our rectangle */
            HRGN hrgn, hrgnOld;
            GpiCreateRegion(hps, 1, &rect);
            GpiSetClipRegion(hps, hrgn, &hrgnOld);

            /* draw using common drawing routine */
            Draw(This, hps, &endPoint, FALSE);

            /* restore PS after drawing and delete created objects */
            GpiDestroyRegion(hps, hrgn);
            GpiDestroyRegion(hps, hrgnOld);
            GpiRestorePS(hps, saveID);
        }
    }
}


//----------------------------------------------------------------------------
// NPP_HandleEvent:
// Mac-only.
//----------------------------------------------------------------------------
int16 NP_LOADDS NPP_HandleEvent(NPP instance, void* event)
{
    NPBool eventHandled = FALSE;
    if (instance == 0   )
        return eventHandled;

    PluginInstance* This = (PluginInstance*) instance->pdata;

    //
    // *Developers*: The "event" passed in is a Macintosh
    // EventRecord*.  The event.what field can be any of the
    // normal Mac event types, or one of the following additional
    // types defined in npapi.h: getFocusEvent, loseFocusEvent,
    // adjustCursorEvent.  The focus events inform your plugin
    // that it will become, or is no longer, the recepient of
    // key events.  If your plugin doesn't want to receive key
    // events, return false when passed at getFocusEvent.  The
    // adjustCursorEvent is passed repeatedly when the mouse is
    // over your plugin; if your plugin doesn't want to set the
    // cursor, return false.  Handle the standard Mac events as
    // normal.  The return value for all standard events is currently
    // ignored except for the key event: for key events, only return
    // true if your plugin has handled that particular key event.
    //

    return eventHandled;
}

//
// Here is a sample subclass function.
//
MRESULT APIENTRY
SubClassFunc(  HWND hWnd,
               ULONG Message,
               MPARAM wParam,
               MPARAM lParam)
{
    PluginInstance *This = GetInstance(hWnd);

    switch(Message) {
    case WM_REALIZEPALETTE:
        WinInvalidateRect(hWnd, 0, TRUE);
        WinUpdateWindow(hWnd);
        return 0;
        break;

    case WM_PAINT:
        {
            /* invalidate the whole window */
            WinInvalidateRect(hWnd, NULL, TRUE);

            /* get PS associated with window  and set to PU_TWIPS coordinates */
            RECTL invalidRect;
            WinBeginPaint(hWnd, This->hps, &invalidRect);

            /* paint a background for the drawing */
            RECTL rect;
            WinQueryWindowRect(hWnd, &rect);
            WinFillRect(This->hps, &rect, CLR_PALEGRAY);

            GpiSetDrawingMode(This->hps, DM_DRAW);
            GpiDrawChain(This->hps);
            WinEndPaint(This->hps);

        return (MRESULT)0;
        }
        break;

    default:
        break;
    }

    return ((PFNWP)This->lpfnOldWndProc)(
                          hWnd,
                          Message,
                          wParam,
                          lParam);
}

void Draw(PluginInstance *This, HPS hps, POINTL *endPoint, BOOL fPrinting)
{
   return;
}


void parseStream(PluginInstance *This, istrstream *str, ULONG len)
{
    IString type;
    int color, xStart, yStart, xEnd, yEnd;
    str->setf(ios::skipws);

    for (int i=0; i<len/TOTAL_WIDTH; i++)
    {
        *str >> type >> color >> xStart >> yStart >> xEnd >> yEnd;

        /* draw the graphic */
        GpiSetColor(This->hps, color);
        POINTL ptl = { xStart, yStart };
        GpiMove(This->hps, &ptl);
        ptl.x = xEnd;
        ptl.y = yEnd;
        if (type == "L") GpiLine(This->hps, &ptl);
        if (type == "R") GpiBox(This->hps, DRO_OUTLINE, &ptl, 0, 0);
        if (type == "F") GpiBox(This->hps, DRO_FILL, &ptl, 0, 0);

        /* simulate slow network with delay */
        DosSleep(DELAY);
    }
}
