//*****************************************************************************
//
// File Name	: 'tsip.h'
// Title		: TSIP (Trimble Standard Interface Protocol) function library
// Author		: Pascal Stang - Copyright (C) 2002
// Created		: 2002.08.27
// Revised		: 2002.08.27
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

#ifndef TSIP_H
#define TSIP_H

/************************** Includes *******************************/

#include "buffer.h"

#include "gps.h"

/************************** Definition des constantes *******************************/

// constants/macros/typdefs
// packet delimiters
#define DLE								0x10
#define ETX								0x03
// packet types
// command packets
#define TSIPCMD_REQUEST_CURRENT_TIME	0x21
#define TSIPCMD_COLD_START				0x1E
#define TSIPCMD_SOFT_RESET				0x25
#define TSIPCMD_SAT_HEALTH				0x38
#define TSIPCMD_STANDBY					0xC0
#define TSIPCMD_FIRMWARE_VERSION_1		0x1C
#define TSIPCMD_FIRMWARE_VERSION		0x1F
#define TSIPCMD_PROTOCOL_CONFIG			0xBC
#define TSIPCMD_RESET_TYPE				0xC0

#define TSIPTYPE_SET_IO_OPTIONS			0x35
#define TSIPTYPE_REQUEST_LAST_FIX       0x8E

// byte 0
#define POS_XYZ_ECEF					0	// outputs 0x42 and 0x83 packets
#define POS_LLA							1	// outputs 0x4A and 0x84 packets
#define POS_ALT							2	// outputs 0x4A/0x84 and 0x8F-17/0x8F-18
#define ALT_REF_MSL						3	// bit cleared = HAE Reference datum
#define POS_DBL_PRECISION				4	// bit cleared = single precision
#define SUPER_PACKETS					5	// 0x8F-17,0x8F-18,0x8F-20
// byte 1
#define VEL_ECEF						0	// outputs 0x43
#define VEL_ENU							1	// outputs 0x56
// byte 2
#define TIME_UTC						0	// 0/1 time format GPS/UTC
// byte 3
#define RAWDATA							0	// outputs 0x5A packets 
#define RAWDATA_FILTER					1	// 0/1 raw data unfiltered/filtered 
#define SIGNAL_DBHZ						3	// 0/1 signal strength in AMU/dBHz

// report packets
#define TSIPTYPE_GPSTIME				0x41
#define TSIPTYPE_POSFIX_XYZ_SP			0x42	// => simple precision
#define TSIPTYPE_VELFIX_XYZ				0x43
#define TSIPTYPE_HEALTH					0x46
#define TSIPTYPE_SATSIGLEVEL			0x47
#define TSIPTYPE_GPSSYSMESSAGE			0x48
#define TSIPTYPE_POSFIX_LLA_SP			0x4A
#define TSIPTYPE_MACHINE_STATUS			0x4B
#define TSIPTYPE_VELFIX_ENU				0x56
#define TSIPTYPE_SAT_SYSTEM_DATA		0x58
#define TSIPTYPE_SATTRACKSTAT			0x5C
#define TSIPTYPE_RAWDATA				0x5A
#define TSIPTYPE_ALL_IN_VIEW			0x6D
#define TSIPTYPE_GPSSUBCODE				0x6F
#define TSIPTYPE_POSFIX_XYZ_DP			0x83
#define TSIPTYPE_POSFIX_LLA_DP			0x84	// => double precision
#define TSIPTYPE_FIRMWARE_VERSION		0x45
#define TSIPTYPE_FIRMWARE_VERSION_1		0x1C
#define TSIPTYPE_REPORT_PACKET          0x8F

#define TSIPTYPE_GPSTIME_LENGTH			11		// 10 + 1
#define TSIPTYPE_POSFIX_XYZ_SP_LENGTH	17		// 16 + 1
#define TSIPTYPE_POSFIX_LLA_SP_LENGTH	21		// 20 + 1
#define TSIPTYPE_VELFIX_ENU_LENGTH		21		// 20 + 1
#define TSIPTYPE_ALL_IN_VIEW_LENGTH		18		// 17 + 1 ( minimum )
#define TSIPTYPE_HEALTH_LENGTH			3		// 2 + 1
#define TSIPTYPE_MACHINE_STATUS_LENGTH  4		// 3 + 1
#define TSIPTYPE_POSFIX_LLA_DP_LENGTH	37		// 36 + 1
#define TSIPTYPE_SAT_SYSTEM_DATA_LENGTH 37

#define TSIPTYPE_REPORT_PACKET_LENGTH   65

/************************** Definition des Fonctions ********************************/

#ifdef __cplusplus
extern "C" {
#endif

extern cBuffer *com_GetRxBuffer(void);
extern int      com_RxOverflow(void);
extern void     com_ResetRxOverflow(void);


GpsInfoType *TSIP_Init(void);
int		     TSIP_Process();
void         TSIP_SendPacket(unsigned char PA_TsipType, unsigned char *PA_Data, int PA_DataLength);

#ifdef __cplusplus
}
#endif

#endif
