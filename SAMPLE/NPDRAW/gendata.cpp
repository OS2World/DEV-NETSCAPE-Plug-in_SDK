/***************************************************************************
 *
 * File name   :  gendata.cpp
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

#include <stdlib.h>
#include <iostream.h>

#include "draw.h"

#define DEFAULT_COUNT 100

main(int argc, char *argv[])
{
    // initialize number of items to output
    int count;
    if (argc > 1)
        count = atoi((const char *)argv[1]);
    else
        count = DEFAULT_COUNT;

    for (int i=0; i<count; i++)
    {
       int type, color, xStart, yStart, xEnd, yEnd;
       char *typeName;

       type = rand() % POLYGON_TYPES;
       color = rand() % NUM_COLORS + 1;
       xStart = rand() % MAX_X;
       yStart = rand() % MAX_Y;
       xEnd = rand() % MAX_X;
       yEnd = rand() % MAX_Y;
       if (type == 0) typeName = "L";  // line
       if (type == 1) typeName = "R";  // rectangle
       if (type == 2) typeName = "F";  // filled rectangle
       cout.width(TYPE_WIDTH);
       cout << typeName;
       cout.width(COLOR_WIDTH);
       cout << color;
       cout.width(COORD_WIDTH);
       cout << xStart;
       cout.width(COORD_WIDTH);
       cout << yStart;
       cout.width(COORD_WIDTH);
       cout << xEnd;
       cout.width(COORD_WIDTH);
       cout << yEnd << endl;
    }
}
