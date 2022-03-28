/*! \file gps.c \brief GPS position storage and processing library. */
//*****************************************************************************
//
// File Name	: 'gps.c'
// Title		: GPS position storage and processing function library
// Author		: Pascal Stang - Copyright (C) 2002-2003
// Created		: 2002.08.29
// Revised		: 2002.07.17
// Version		: 0.1
// Target MCU	: Atmel AVR Series
// Editor Tabs	: 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

/************************** Includes *******************************/

#include <math.h>
#include <string.h>

#include "gps.h"


/************************** Constants definition  ********************************/

/************************** Variables definition  *******************************/

static GpsInfoType	GpsInfo = { 0 };

/************************** Function Definition    ********************************/


/************************** Functions ********************************************/


/**********************************************************************/
/*! \brief		Get GpsInfos structure pointer
 *  \return     Pointer to GpsInfos structure
 */
/**********************************************************************/
GpsInfoType* GPS_GetInfo(void)
{
	return &GpsInfo;
}
