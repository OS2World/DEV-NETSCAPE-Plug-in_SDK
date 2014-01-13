/***************************************************************************
 *
 * File name   :  npvideo.cpp
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
#define INCL_DOSERRORS
#include <os2.h>

#include "video.h"
#include "video.hpp"



/*------------------------------------------------------------------------------
| VIDEO::VIDEO                                                                   |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
VIDEO::VIDEO( IMMDigitalVideo*  aVideoPlayer,
              ISize             windowSize,
              unsigned long     windowid,
              IString           filename,
              IWindow*          parent,
              IWindow*          owner)
     : IMMPlayerPanel(windowid,parent,owner,IMMDevice::digitalVideo),
       pVideoPlayer  (aVideoPlayer),
       videoCanvas   (VIDEOID,this,this),
            volupbtn        (VOLUPID ,this,this,
                            IRectangle(IPoint(0,0), ISize(36,40)),
                            IControl::group |
                            IWindow::visible | IAnimatedButton::animateWhenLatched),
            voldnbtn        (VOLDNID ,this,this,
                            IRectangle(IPoint(0,0), ISize(36,40)),
                            IWindow::visible | IAnimatedButton::animateWhenLatched),
            flyText(FLYTEXTID, this),
            flyHelpHandler(&flyText, 100, 5),

       observer      (0),
       handler       (0)
{
   // load video and get file size
   setPlayableDevice(pVideoPlayer);
   pVideoPlayer->loadOnThread(filename);
   pVideoPlayer->startPositionTracking(IMMTime(3000));
   pVideoPlayer->setWindow(videoCanvas.handle());
   int videoWidth = pVideoPlayer->videoFileWidth();
   int videoHeight = pVideoPlayer->videoFileHeight();

   // set bitmaps for volume buttons
   volupbtn.setBitmaps(IAnimatedButton::volumeUp);
   voldnbtn.setBitmaps(IAnimatedButton::volumeDown);

   // change position of standard buttons
   addToCell(removeFromCell(1,1), 1, 3);
   addToCell(removeFromCell(2,1), 2, 3);
   addToCell(removeFromCell(3,1), 3, 3);
   addToCell(removeFromCell(4,1), 4, 3);
   addToCell(removeFromCell(5,1), 5, 3);
   addToCell(removeFromCell(6,1), 6, 3);
   addToCell(removeFromCell(7,1), 7, 3);

   // add additional buttons
   addToCell(&voldnbtn,  9, 3);
   addToCell(&volupbtn, 10, 3);

   // set column and row widths
   setDefaultCell(ISize(MARGIN, MARGIN));
   setColumnWidth(1, MARGIN, true);
   setColumnWidth(2, MARGIN, true);
   setColumnWidth(3, MARGIN, true);
   setColumnWidth(4, MARGIN, true);
   setColumnWidth(5, MARGIN, true);
   setColumnWidth(6, MARGIN, true);
   setColumnWidth(7, MARGIN, true);
   setColumnWidth(8, MARGIN, true);
   setColumnWidth(9, MARGIN, true);
   setColumnWidth(10, MARGIN, true);
   setColumnWidth(11, MARGIN, true);
   setRowHeight(1, MARGIN, true);
   setRowHeight(2, MARGIN, true);
   setRowHeight(3, MARGIN, true);
   setRowHeight(4, MARGIN, true);

   // calculate size of video canvas and add to multicell canvas
#ifdef NEW
   videoCanvas.setMinimumSize(windowSize);
   videoCanvas.sizeTo(windowSize);
#else
   videoCanvas.setMinimumSize(ISize(200,200));
   videoCanvas.sizeTo(ISize(200,200));
#endif
   videoCanvas.setBackgroundColor(IColor(IColor::paleGray));
   addToCell(&videoCanvas, 1, 1, 11, 1);

   // setup event handler and observer
   if (pVideoPlayer)
   {
      handler  = new VideoHandler();
      handler->handleEventsFor(this);
      observer = new VideoObserver(*this);
      observer->handleNotificationsFor(*pVideoPlayer);
   }

   videoCanvas.show();
   videoCanvas.setFocus();
   videoCanvas.refresh();

//*****************************************************************************
//  Set up the fly over help handler                                          *
//*****************************************************************************

   char strError[CCHMAXPATH];
   APIRET rc = DosLoadModule(strError, CCHMAXPATH, "npvideo", &hmod);
   if (rc == NO_ERROR)
       flyHelpHandler.setResourceLibrary(IModuleHandle(hmod));
   else
       hmod = 0;
   flyHelpHandler.handleEventsFor(this);
}


/*------------------------------------------------------------------------------
| VIDEO::~VIDEO                                                                      |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
VIDEO::~VIDEO()
{
  if (observer)
     observer->stopHandlingNotificationsFor(*pVideoPlayer);
  if (handler)
     handler->stopHandlingEventsFor(this);
  if (!hmod)
     DosFreeModule(hmod);
}


/*------------------------------------------------------------------------------
| VIDEO::videoPlayer                                                             |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
IMMDigitalVideo* VIDEO::videoPlayer () const
{
  return pVideoPlayer;
}


/*------------------------------------------------------------------------------
| VideoHandler::VideoHandler                                                     |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
VideoHandler::VideoHandler()
          : VideoHandler::Inherited()
{}

/*------------------------------------------------------------------------------
| VideoHandler::command                                                         |
|                                                                              |
------------------------------------------------------------------------------*/
Boolean VideoHandler::command(ICommandEvent& event)
{
  Boolean handled = false;
  VIDEO* panel = (VIDEO *) (event.window());

  switch (event.commandId())
  {
//*****************************************************************************
//  Turn down volume 5%                                                       *
//*****************************************************************************

  case VOLDNID:
       panel->videoPlayer()->setVolume(panel->videoPlayer()->volume()-5);
       handled = true;
       break;

//*****************************************************************************
//  Turn up volume 5%                                                         *
//*****************************************************************************

  case VOLUPID:
       panel->videoPlayer()->setVolume(panel->videoPlayer()->volume()+5);
       handled = true;
       break;
    default:
         handled = Inherited::command(event);
         break;
  } /* endswitch */
  return handled;
}


/*------------------------------------------------------------------------------
| VideoObserver::VideoObserver                                                   |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
VideoObserver::VideoObserver(VIDEO& videoPanel)
          : VideoObserver::Inherited(),
          panel(videoPanel)
{}

/*------------------------------------------------------------------------------
| VideoObserver::dispatchNotificationEvent                                      |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
VideoObserver& VideoObserver::dispatchNotificationEvent(const INotificationEvent& event)
{
   if (event.notificationId() == IMMDevice::positionChangeId)
   {
      IMMPositionChangeEvent* positionEvent = (IMMPositionChangeEvent*)(event.eventData().asUnsignedLong());
      IMMTime time(positionEvent->position());

      if (!panel.playButton()->isLatched())
         if (panel.videoPlayer()->mode()==IMMDevice::playing)
            panel.playButton()->latch();
   } /* endif */

   if (event.notificationId() == IMMDevice::commandNotifyId)
   {
      IMMNotifyEvent* notifyEvent = (IMMNotifyEvent*)(event.eventData().asUnsignedLong());
      if (notifyEvent->command() == IMMNotifyEvent::play)
         panel.playButton()->unlatch();
   } /* endif */
   return *this;
}
