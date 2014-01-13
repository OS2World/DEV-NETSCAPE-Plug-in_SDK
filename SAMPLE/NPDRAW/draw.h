/***************************************************************************
 *
 * File name   :  draw.h
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

#define NUM_COLORS 15
#define MAX_X 100  // all coordinates are normalized to 0-99
#define MAX_Y 100

#define POLYGON_TYPES 3

#define TYPE_WIDTH 1
#define COLOR_WIDTH 5
#define COORD_WIDTH 4

#define TOTAL_WIDTH   (TYPE_WIDTH + COLOR_WIDTH + 4 * COORD_WIDTH + 2)

#define DELAY 10
