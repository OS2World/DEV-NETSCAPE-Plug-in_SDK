/***************************************************************************
 *
 * File name   :  audio.hpp
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

#include <istring.hpp>
#include <immwave.hpp>
#include <ifont.hpp>
#include <inotifev.hpp>
#include <icolor.hpp>
#include <ianimbut.hpp>
#include <icmdhdr.hpp>
#include <iobservr.hpp>
#include <immplypn.hpp>
#include <immplyhd.hpp>
#include <iflytext.hpp>
#include <iflyhhdr.hpp>

#include <os2def.h>  // needed for HMODULE

#include "audio.h"

class AudioHandler;
class AudioObserver;

class AUDIO  : public IMMPlayerPanel {
//**************************************************************************
// Class:   AUDIO                                                          *
//                                                                         *
// Purpose: Provide a Audio player.                                        *
//          It is a subclass of IMMPlayerPanel                             *
//                                                                         *
//**************************************************************************
public:

  AUDIO( IMMWaveAudio*     pAudio,
         ISize             windowSize,
         unsigned long     windowid,
         IString           filename,
         IWindow*          parent,
         IWindow*          owner);

  ~AUDIO();

  IMMWaveAudio* audioPlayer          () const;

  IAnimatedButton* playButton  () const;

  Boolean isPlaying                  () const;

  void togglePlay();

private:
IMMWaveAudio
  *pAudioPlayer;

IAnimatedButton
  playbtn,
  volupbtn,
  voldnbtn;

IFlyText
  flyText;

IFlyOverHelpHandler
  flyHelpHandler;

AudioHandler
  *handler;

AudioObserver
  *observer;

HMODULE hmod;

Boolean canPlay;

};

class AudioHandler : public IMMPlayerPanelHandler {
typedef IMMPlayerPanelHandler
  Inherited;
//**************************************************************************
// Class:   AudioHandler                                                   *
//                                                                         *
// Purpose: Provide a Handler for processing the load button.              *
//                                                                         *
//**************************************************************************
public:

  AudioHandler ();

/*---------------------------- Event Dispatching -----------------------------*/
virtual Boolean
  command (ICommandEvent& event);

};

class AudioObserver : public IObserver {
typedef IObserver
  Inherited;
//**************************************************************************
// Class:   AudioObserver                                                  *
//                                                                         *
// Purpose: Provide an Observer for processing the play notifications.     *
//                                                                         *
//**************************************************************************
public:

  AudioObserver (AUDIO& audioPanel);

virtual AudioObserver
  &dispatchNotificationEvent(const INotificationEvent&);

private:
AUDIO
  &panel;

};
