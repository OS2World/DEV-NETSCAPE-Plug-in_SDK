/***************************************************************************
 *
 * File name   :  clock.hpp
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

#ifndef _CLOCK_HPP_
#define _CLOCK_HPP_

#ifndef _CLOCK_
#include "Clock.h"
#endif

class Clock
{
public:
       Clock();
       ~Clock();
  HRGN CreateRegion( HPS hps );
  HRGN CreateFaceRegion( HPS hps );
  void Size( RECTL& rcl, HRGN *hrgnUsed);
  void Draw( HPS hps, PRECTL rclFrame, BOOL toScreen, BOOL fTransparent );
  void DrawAnalog(HPS hps, BOOL fTransparent);
  void DrawDigitalTime(HPS hps, RECTL& rcl, BOOL fTransparent);
  BOOL Timer( HPS hps, PRECTL rclFrame);
  void UpdateScreen(HPS hps, PRECTL rclFrame);
  void DrawHand( HPS hps , SHORT sHandType , SHORT sAngle, LONG vclrFace[], LONG vclrHands[] );
  void DrawFace( HPS hps, LONG vclrFace[] );
  void DrawRing( HPS hps, LONG vclrRing[], LONG vclrFace[] );
  void DrawFullCircle( HPS hps, PPOINTL pptlCenter, FIXED fxRad);
  void DrawDate( HPS hps, USHORT usDrawMode);
  void GetArrangedDate( CHAR achFinalDate[]);
  void DrawTicks ( HPS hps , USHORT usTicks, LONG vclrMajorTicks[], LONG vclrMinorTicks[] );
  void DrawTrapez( HPS hps,POINTL *aptl,LONG color);
  void DrawFullRing( HPS hps,PPOINTL pptlCenter,FIXED fxRadIn, FIXED fxRadOut, LONG lColor);
  void SetRGBColors();
  void ShadeLight( PLONG nplColors);
  BYTE ShadeRGBByte( BYTE brgb);
  BYTE LightRGBByte( BYTE brgb);
  void FillRect(HPS hps, RECTL & rcl);
  void SetMode(USHORT dispMode);

private:
  CLKDATA data;
  RECTL rcl;
  RECTL rclPage;

};
#endif
