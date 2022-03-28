/*! \file gps.h \brief GPS position storage and processing library. */
//*****************************************************************************
//
// File Name	: 'gps.h'
// Title		: GPS position storage and processing function library
// Author		: Pascal Stang - Copyright (C) 2002
// Created		: 2002.08.29
// Revised		: 2002.08.29
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

#ifndef _GPS_H
#define _GPS_H

#include <stdint.h>

/************************** Type definition **************************************/

typedef union union_float_u32
{
	float			f;
	uint32_t     	i;
	unsigned char	b[4];
} float_u32;

struct PositionLLA
{
	float_u32		lat;
	float			LatDeg;
	
	float_u32		lon;
	float			LongDeg;
	
	float_u32		alt;
	long			altLong;

	float_u32		TimeOfFix;
	int				updates;
};

struct GPSModeInfos
{	
	int				GpsModeValue;
	int				GpsModeType;
	int				nSVs;
	float_u32		PDOP;
	float_u32		HDOP;
	float_u32		VDOP;
	float_u32		TDOP;
};

//----------------------------------------------------------

typedef struct struct_GpsInfo
{	
	float_u32				TimeOfWeek;
	int						WeekNum;
	float_u32				UtcOffset;
	
	struct PositionLLA		PosLLA;
	struct GPSModeInfos 	GPSModeInfos;
	int						GPSHealthStatus;

	int                     vE;
	int                     vN;
	int                     vU;

} GpsInfoType;

/************************** Functions definition  ********************************/

GpsInfoType			*GPS_GetInfo(void);

#endif
