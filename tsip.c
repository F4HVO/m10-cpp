

/************************** Includes *******************************/

#include <msp430.h>

#include <math.h>
#include <string.h>

#include "tsip.h"
#include "gps.h"

/************************** Constants definition *******************************/

#define	BUFFERSIZE				0x60

/************************** Type definition      *******************************/

/************************** Variables definition *******************************/

unsigned char				tsipPacketSend[BUFFERSIZE];
unsigned char               tsipPacketReceive[BUFFERSIZE];

static GpsInfoType			*gpsInfo = NULL;

/************************** Functions definition ********************************/

int				TSIP_ProcessGPSTIME(unsigned char *packet,int );
int				TSIP_ProcessPOSFIX_LLA_SP(unsigned char *packet,int);
int				TSIP_ProcessPOSFIX_LLA_DP(unsigned char *packet,int);
int				TSIP_ProcessAlIN_VIEW(unsigned char *packet,int);

int             TSIP_ProcessReportPacket(unsigned char *packet,int);


//! returns pointer to the receive buffer structure
static cBuffer	*uartGetRxBuffer(void);
//! returns overflow count
static int		uartGetOverFlow(void);
static void		uartResetOverFlow(void);

/************************** Functions *******************************************/

GpsInfoType *TSIP_Init(void)
{
	gpsInfo = GPS_GetInfo(); // Init structure GPSInfos address

	return(gpsInfo);
}

void TSIP_SendPacket(unsigned char TsipType,unsigned char *Data, int DataLength )
{
	unsigned int i;
	unsigned int DataIdx;

	//--------------------------------------------------------

	DataIdx = 0;

	// start of packet
	tsipPacketSend[DataIdx++] = DLE;
	// packet type
	tsipPacketSend[DataIdx++] = TsipType;

	if ( Data != NULL )
	{
		// add packet data
		for ( i = 0 ;  ( i < DataLength ) && ( DataIdx < ( sizeof(tsipPacketSend) - 4 ) ) ; i++ )
		{
			if ( *Data == DLE )
			{
				// do double-DLE escape sequence
				tsipPacketSend[DataIdx++] = *Data;
				tsipPacketSend[DataIdx++] = *Data++;
			}
			else
			{
				tsipPacketSend[DataIdx++] = *Data++;
			}
		}
	}
	
	// end of packet
	tsipPacketSend[DataIdx++] = DLE;
	tsipPacketSend[DataIdx++] = ETX;

	//-------------------------------------------------------------

	// Send packet
	for ( i = 0 ; i < DataIdx ; i++ )
	{
	    while ((UCA0STAT & UCBUSY));

	    UCA0TXBUF = tsipPacketSend[i];
	}
}

/**********************************************************************/
/*! \brief		process the TSIP frames
 *  \return		0 : TSIP frame not find , 1 : find a TSIP frame , 2 : find a double precision TSIP frame
 */
/**********************************************************************/
int TSIP_Process()
{
	int				findFrame;
	int				startFlag;
	
	int				i;
	int				j;

	unsigned int    tsipPacketIdx;
	int				Taille;
	unsigned char	datas;

	cBuffer			*rxBuffer;

	int				result;
	int				nbreDLE;

	//------------------------------------------------------------------

	findFrame = 0;

	startFlag = 0;

	rxBuffer = uartGetRxBuffer();

	tsipPacketIdx = 0;

	i = 0;

	//-----------------------------------------------------------------
	// Find a TSIP frame
	//-----------------------------------------------------------------
	Taille = bufferLength(rxBuffer);

	while ( Taille > 1 )
	{
		datas = bufferGetAtIndex(rxBuffer,0);

		// Find start TSIP frame
		if ( datas == DLE )
		{
			// Next character
			datas = bufferGetAtIndex(rxBuffer,1);

			// Check character DLE ou ETX
			if ( ( datas != DLE ) && ( datas != ETX ) )
			{
				// Find start of TSIP frame
				startFlag = 1;
				break;
			}
			else
			{
				// pop the character
				bufferGetFromFront(rxBuffer);
			}
		}
		else
		{
			// not a DLE character  the pop the character
			bufferGetFromFront(rxBuffer);
		}

		//--------------------------------
		// Check buffer overflow
		//--------------------------------
		if ( uartGetOverFlow() > 0 )
		{
			bufferFlush( uartGetRxBuffer() );
			uartResetOverFlow();
			
			break;
		}

		//--------------------------------

		Taille = bufferLength(rxBuffer);
	}

	//--------------------------------------------------------------------
	// Find start of TSIP frame , the find the end
	//--------------------------------------------------------------------
	if ( startFlag == 1 )
	{
		Taille = bufferLength(rxBuffer);

		findFrame = 0;

		nbreDLE = 0;

		i = 1;

		while (	i < ( Taille - 1 ) && ( findFrame == 0 ) )
		{
			// Find the end of TSIP frame
			if ( bufferGetAtIndex(rxBuffer,i) == DLE )
			{
				nbreDLE++;

				if  ( 
				      ( bufferGetAtIndex(rxBuffer,i+1) == ETX ) &&
					  ( ( nbreDLE & 0x01 ) == 0x01 )
					)
				{
					bufferGetFromFront(rxBuffer);

					tsipPacketIdx = 0;

					memset(tsipPacketReceive,0,sizeof(tsipPacketReceive));

					j = 0;

					while ( j < ( i - 1 ) )
					{
						// Next Character
						datas = bufferGetFromFront(rxBuffer);

						// Check DLE character
						if ( datas == DLE )
						{
							// Check the next character
							if ( bufferGetAtIndex(rxBuffer,0) == DLE )
							{
								// suppress the second DLE character
								bufferGetFromFront(rxBuffer);
								j++;
							}
						}

						// Check buffer size
						if ( tsipPacketIdx < sizeof(tsipPacketReceive) )
						{
							tsipPacketReceive[tsipPacketIdx++] = datas; // memoriser le caractere
						}
						else
						{
						    // Buffer Overrun
						}

						j++;
					}

					// Read end characters   ( DLE + ETX )
					bufferGetFromFront(rxBuffer);
					bufferGetFromFront(rxBuffer);

					findFrame = 1;
				} // End If
	 		}
			else
			{
				// init counter
				nbreDLE = 0;
			}
	
			//--------------------------------
			// Overflow check
			//--------------------------------
			if ( uartGetOverFlow() > 0 )
			{
				// RAZ GPS buffer
				bufferFlush( uartGetRxBuffer() );
				uartResetOverFlow();
				break;
			}

			// next character
			i++;
		} // End while
	} // end If

	if ( findFrame == 1 )
	{
		result = 0;

		switch ( tsipPacketReceive[0] )
		{
			//---------------------------------------------
			case TSIPTYPE_GPSTIME:
			{
			    findFrame = TSIP_ProcessGPSTIME(tsipPacketReceive,tsipPacketIdx);
			}
			break;
			
			//---------------------------------------------
			case TSIPTYPE_POSFIX_LLA_SP:
			{
				result = TSIP_ProcessPOSFIX_LLA_SP(tsipPacketReceive,tsipPacketIdx);
			}
			break;

			//---------------------------------------------
			case TSIPTYPE_POSFIX_LLA_DP:
			{
				result = TSIP_ProcessPOSFIX_LLA_DP(tsipPacketReceive,tsipPacketIdx);

				findFrame = 2;
			}
			break;

			//---------------------------------------------
			case TSIPTYPE_ALL_IN_VIEW:
			{
				result = TSIP_ProcessAlIN_VIEW(tsipPacketReceive,tsipPacketIdx);

				if (result == 3) {
				    findFrame = 4;
				}
			}
			break;

            //---------------------------------------------
			case TSIPTYPE_REPORT_PACKET:
			{
	            TSIP_ProcessReportPacket(tsipPacketReceive,tsipPacketIdx);
			}
			break;

			//---------------------------------------------
			default:
			{
				result = 0;
			}
			break;
		}

	}

	return findFrame;
}

/**********************************************************************/

int TSIP_ProcessAlIN_VIEW(unsigned char *packet,int Lg)
{
	if ( Lg >= TSIPTYPE_ALL_IN_VIEW_LENGTH )
	{
		if ( gpsInfo->GPSHealthStatus == 0x00 )
		{
			gpsInfo->GPSModeInfos.GpsModeValue = ( packet[1] & 0x07 ) - 1;
		}
		else
		{
			gpsInfo->GPSModeInfos.GpsModeValue = 0;							// 0d
		}

		if ( gpsInfo->GPSModeInfos.GpsModeValue == 3 )
		{
		    return(3);
		}
		else
		{
		    return(1);
		}
	}
	else
	{
		return(0);
	}
}


/**********************************************************************/

int TSIP_ProcessGPSTIME(unsigned char *packet,int Lg)
{
    if ( Lg == TSIPTYPE_GPSTIME_LENGTH )
    {
        // NOTE: check endian-ness
        gpsInfo->TimeOfWeek.b[3] = packet[1];
        gpsInfo->TimeOfWeek.b[2] = packet[2];
        gpsInfo->TimeOfWeek.b[1] = packet[3];
        gpsInfo->TimeOfWeek.b[0] = packet[4];

        gpsInfo->WeekNum	= ((int)packet[5]<<8)|((int)packet[6]);

        return(3);
    }
	else
	{
		return(0);
	}
}

/**********************************************************************/

int TSIP_ProcessPOSFIX_LLA_SP(unsigned char *packet,int Lg)
{
	if ( Lg == TSIPTYPE_POSFIX_LLA_SP_LENGTH )
	{
        if ( gpsInfo->GPSModeInfos.GpsModeValue == 3 ) {
            return(3);
        }
        else {
            return(1);
        }
	}
	else
	{
		return(0);
	}
}

/**********************************************************************/

int TSIP_ProcessPOSFIX_LLA_DP(unsigned char *packet,int Lg)
{
	if ( Lg == TSIPTYPE_POSFIX_LLA_DP_LENGTH )
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

/**********************************************************************/

int TSIP_ProcessReportPacket(unsigned char *packet,int Lg)
{
    int value = 0;

    if ( Lg == TSIPTYPE_REPORT_PACKET_LENGTH ) {

        if ( packet[1] == 0x20 ) {

            gpsInfo->PosLLA.lat.b[3] = packet[13];
            gpsInfo->PosLLA.lat.b[2] = packet[14];
            gpsInfo->PosLLA.lat.b[1] = packet[15];
            gpsInfo->PosLLA.lat.b[0] = packet[16];

            gpsInfo->PosLLA.lon.b[3] = packet[17];
            gpsInfo->PosLLA.lon.b[2] = packet[18];
            gpsInfo->PosLLA.lon.b[1] = packet[19];
            gpsInfo->PosLLA.lon.b[0] = packet[20];


            gpsInfo->PosLLA.alt.b[3] = packet[21];
            gpsInfo->PosLLA.alt.b[2] = packet[22];
            gpsInfo->PosLLA.alt.b[1] = packet[23];
            gpsInfo->PosLLA.alt.b[0] = packet[24];

            gpsInfo->PosLLA.altLong = (long)(gpsInfo->PosLLA.alt.i) / 1000;


            gpsInfo->GPSModeInfos.nSVs = packet[29];

            gpsInfo->UtcOffset.b[3] = 0;
            gpsInfo->UtcOffset.b[2] = 0;
            gpsInfo->UtcOffset.b[1] = 0;
            gpsInfo->UtcOffset.b[0] = packet[30];

            gpsInfo->vE = ((int)packet[3]<<8)|((int)packet[4]);
            gpsInfo->vN = ((int)packet[5]<<8)|((int)packet[6]);
            gpsInfo->vU = ((int)packet[7]<<8)|((int)packet[8]);

            value = 1;
        }
    }

    return(value);
}

/**********************************************************************/
/*!
 *  \brief		Fonction de lecture de l'adresse du buffer de reception
 *  \ingroup	TSIP_functions
 *  \return		pointeur sur le buffer
 */
/**********************************************************************/
cBuffer *uartGetRxBuffer()
{
	// return rx buffer pointer
	return com_GetRxBuffer();
}

/**********************************************************************/
/*!
 *  \brief		Fonction de lecture de l'indicateur de debordement
 *  \return		
 */
/**********************************************************************/
int uartGetOverFlow()
{
	return com_RxOverflow();
}

/**********************************************************************/
/*!
 *  \brief		Fonction de RAZ de l'indicateur de debordement

 */
/**********************************************************************/
void uartResetOverFlow()
{
	com_ResetRxOverflow();
}

