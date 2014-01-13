/***************************************************************************
 *
 * File name   :  audio.cpp
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

#include <ireslib.hpp>

#include "audio.h"
#include "audio.hpp"


/*------------------------------------------------------------------------------
| AUDIO::AUDIO                                                                   |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
AUDIO::AUDIO( IMMWaveAudio*     aAudioPlayer,
              ISize             windowSize,
              unsigned long     windowid,
              IString           filename,
              IWindow*          parent,
              IWindow*          owner)
     : IMMPlayerPanel(windowid,parent,owner,IMMDevice::waveAudio),
       pAudioPlayer  (aAudioPlayer),
            playbtn         (PLAYSTOPID ,this,this,
                            IRectangle(IPoint(0,0), ISize(36,40)),
                            IControl::group |
                            IWindow::visible | IAnimatedButton::animateWhenLatched),
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
   // load audio and get file size
   if (pAudioPlayer)
   {
       setPlayableDevice(pAudioPlayer);
       pAudioPlayer->loadOnThread(filename);
       pAudioPlayer->startPositionTracking(IMMTime(3000));
   }

   // set bitmaps for volume buttons
   playbtn.setBitmaps(IResourceId(PLAYBITMAPID,
                                  IDynamicLinkLibrary("npaudio")),
                      2);
   volupbtn.setBitmaps(IAnimatedButton::volumeUp);
   voldnbtn.setBitmaps(IAnimatedButton::volumeDown);

   // remove standard buttons
   removeFromCell(1,1);
   removeFromCell(2,1);
   removeFromCell(3,1);
   removeFromCell(4,1);
   removeFromCell(5,1);
   removeFromCell(6,1);
   removeFromCell(7,1);

   // add additional buttons
   addToCell(&playbtn, 1, 1, 2, 2);
   addToCell(&volupbtn, 3, 1);
   addToCell(&voldnbtn, 3, 2);

   // set column and row widths
   setDefaultCell(ISize(MARGIN, MARGIN));
   setColumnWidth(1, MARGIN, false);
   setColumnWidth(2, MARGIN, false);
   setColumnWidth(3, MARGIN, false);
   setRowHeight(1, MARGIN, false);
   setRowHeight(2, MARGIN, false);

   // setup event handler and observer
   handler  = new AudioHandler();
   handler->handleEventsFor(this);
   if (pAudioPlayer)
   {
      observer = new AudioObserver(*this);
      observer->handleNotificationsFor(*pAudioPlayer);
   }

//*****************************************************************************
//  Set up the fly over help handler                                          *
//*****************************************************************************

#ifdef BAD
   char strError[CCHMAXPATH];
   APIRET rc = DosLoadModule(strError, CCHMAXPATH, "plugins\\npaudio", &hmod);
   if (rc == NO_ERROR)
       flyHelpHandler.setResourceLibrary(IModuleHandle(hmod));
   else
       hmod = 0;
#endif
   flyHelpHandler.handleEventsFor(this);

   // set boolean for play button state
   canPlay = true;
}


/*------------------------------------------------------------------------------
| AUDIO::~AUDIO                                                                      |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
AUDIO::~AUDIO()
{
  if (pAudioPlayer)
     if (observer)
        observer->stopHandlingNotificationsFor(*pAudioPlayer);
  if (handler)
     handler->stopHandlingEventsFor(this);
  if (!hmod)
     DosFreeModule(hmod);
}


/*------------------------------------------------------------------------------
| AUDIO::audioPlayer                                                             |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
IMMWaveAudio* AUDIO::audioPlayer () const
{
  return pAudioPlayer;
}



IAnimatedButton* AUDIO::playButton () const
{
   return (IAnimatedButton*)&playbtn;
}

Boolean AUDIO::isPlaying () const
{
   return ! canPlay;
}

void AUDIO::togglePlay()
{
   if (canPlay)
       canPlay = false;
   else
       canPlay = true;
}


/*------------------------------------------------------------------------------
| AudioHandler::AudioHandler                                                     |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
AudioHandler::AudioHandler()
          : AudioHandler::Inherited()
{}

/*------------------------------------------------------------------------------
| AudioHandler::command                                                         |
|                                                                              |
------------------------------------------------------------------------------*/
Boolean AudioHandler::command(ICommandEvent& event)
{
  Boolean handled = false;
  AUDIO* panel = (AUDIO *) (event.window());
  IMMWaveAudio* wavePlayer = panel->audioPlayer();

  switch (event.commandId())
  {
//*****************************************************************************
//  Play/Stop audio                                                           *
//*****************************************************************************

  case PLAYSTOPID:
       if (! panel->isPlaying())
       {
          if (wavePlayer)
              wavePlayer->play();
          panel->playButton()->setCurrentBitmapIndex(1);
       }
       else
       {
          if (wavePlayer)
          {
              wavePlayer->stop();
              wavePlayer->seekToStart();
          }
          panel->playButton()->setCurrentBitmapIndex(0);
       }
       panel->togglePlay();
       handled = true;
       break;

//*****************************************************************************
//  Turn down volume 5%                                                       *
//*****************************************************************************

  case VOLDNID:
       if (wavePlayer)
           wavePlayer->setVolume(wavePlayer->volume()-5);
       handled = true;
       break;

//*****************************************************************************
//  Turn up volume 5%                                                         *
//*****************************************************************************

  case VOLUPID:
       if (wavePlayer)
           wavePlayer->setVolume(wavePlayer->volume()+5);
       handled = true;
       break;

    default:
         handled = Inherited::command(event);
         break;
  } /* endswitch */
  return handled;
}


/*------------------------------------------------------------------------------
| AudioObserver::AudioObserver                                                   |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
AudioObserver::AudioObserver(AUDIO& audioPanel)
          : AudioObserver::Inherited(),
          panel(audioPanel)
{}

/*------------------------------------------------------------------------------
| AudioObserver::dispatchNotificationEvent                                      |
|                                                                              |
|                                                                              |
------------------------------------------------------------------------------*/
AudioObserver& AudioObserver::dispatchNotificationEvent(const INotificationEvent& event)
{
   if (event.notificationId() == IMMDevice::positionChangeId)
   {
      IMMPositionChangeEvent* positionEvent = (IMMPositionChangeEvent*)(event.eventData().asUnsignedLong());
      IMMTime time(positionEvent->position());

      if (!panel.playButton()->isLatched())
         if (panel.audioPlayer()->mode()==IMMDevice::playing)
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