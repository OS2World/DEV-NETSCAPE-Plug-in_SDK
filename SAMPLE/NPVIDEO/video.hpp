/***************************************************************************
 *
 * File name   :  video.cpp
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
#include <immdigvd.hpp>
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

#include "video.h"

class VideoHandler;
class VideoObserver;

class VIDEO  : public IMMPlayerPanel {
//**************************************************************************
// Class:   VIDEO                                                          *
//                                                                         *
// Purpose: Provide a Video player.                                        *
//          It is a subclass of IMMPlayerPanel                             *
//                                                                         *
//**************************************************************************
public:

  VIDEO( IMMDigitalVideo*  pVideo,
         ISize             windowSize,
         unsigned long     windowid,
         IString           filename,
         IWindow*          parent,
         IWindow*          owner);

  ~VIDEO();

  ICanvas &getVideoCanvas()    { return videoCanvas; };

  IMMDigitalVideo* videoPlayer          () const;


private:
IMMDigitalVideo
  *pVideoPlayer;

ICanvas
  videoCanvas;

IAnimatedButton
  volupbtn,
  voldnbtn;

IFlyText
  flyText;

IFlyOverHelpHandler
  flyHelpHandler;

VideoHandler
  *handler;

VideoObserver
  *observer;

HMODULE hmod;

};

class VideoHandler : public IMMPlayerPanelHandler {
typedef IMMPlayerPanelHandler
  Inherited;
//**************************************************************************
// Class:   VideoHandler                                                   *
//                                                                         *
// Purpose: Provide a Handler for processing the load button.              *
//                                                                         *
//**************************************************************************
public:

  VideoHandler ();

/*---------------------------- Event Dispatching -----------------------------*/
virtual Boolean
  command (ICommandEvent& event);

};

class VideoObserver : public IObserver {
typedef IObserver
  Inherited;
//**************************************************************************
// Class:   VideoObserver                                                  *
//                                                                         *
// Purpose: Provide an Observer for processing the play notifications.     *
//                                                                         *
//**************************************************************************
public:

  VideoObserver (VIDEO& videoPanel);

virtual VideoObserver
  &dispatchNotificationEvent(const INotificationEvent&);

private:
VIDEO
  &panel;

};
