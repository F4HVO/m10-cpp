/*
 * main.cpp
 *
 *  Created on: 27 Apr 2020
 *      Author: hugo
 */

#include <msp430.h>
#include "M10Configuration.h"
#include "RadioAdf7012.h"
#include "M10Packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "buffer.h"
#include "tsip.h"
#include "GTopGPS.h"
#include "M10Data.h"

// Binary packet to send over the air
unsigned char rawPacket[250];
uint32_t rawPacketSize = sizeof(rawPacket);

// Handle radio configuration and operation
RadioAdf7012 radio;

#define RBUF_SIZE 128

static volatile int         uartRxOverflow;
static unsigned char        uartRxData[RBUF_SIZE];
static cBuffer              uartRxBuffer;

#ifdef M10TRIMBLE
static unsigned char        PacketGpsSetIO[4]    = { 0x22 , 0x02 , 0x00 , 0x08 }; // Super packet output + single precision
static unsigned char        SuperPacketConfig[2] = { 0x20 , 0x01 };    // Automatic report

static Position position;
static Speed    speed;
static Datation date;
#else
// Gps frame decoder
GTopGPS gps ;
#endif

//----------------------------------------------------

cBuffer *com_GetRxBuffer(void)
{
    return &uartRxBuffer;
}

int com_RxOverflow(void)
{
    return (uartRxOverflow);
}

void com_ResetRxOverflow(void)
{
    uartRxOverflow = 0;
}

//----------------------------------------------------

int main()
{

    memset(uartRxData,0,sizeof(uartRxData));
    bufferInit(&uartRxBuffer, uartRxData, RBUF_SIZE);

#ifdef M10TRIMBLE
    int ConfigGPS = 0;
    int GpsHeure = 0;
    int result = 0;
    GpsInfoType *ptrGPpsInfos = TSIP_Init();
#else
    gps.setBuffer( com_GetRxBuffer() ) ;
#endif

    // Setup board
    M10::setup();

    // Keep board powered when user releases power button
    M10::mainPower( true ) ;

    // Turn on LED
    M10::digitalWrite( M10::LED, M10::LOW ) ;

    // Blink led to indicate startup
    for ( unsigned char i = 12 ; i > 0 ; --i )
    {
        M10::toggleLed() ;
        M10::delay(100) ;
    }

    // Setup button interrupt for power off
    M10::setupPowerOff() ;

    // Turn on GPS
    M10::gpsPower( true ) ;

    __bis_SR_register( GIE ) ;

    TACCR0 = 0;

    // Main loop

    while ( true )
    {

#ifdef M10TRIMBLE
        result = TSIP_Process();

        if ( result == 2 ) {
            // => configuration du GPS en simple precision + request last fix
            TSIP_SendPacket(TSIPTYPE_SET_IO_OPTIONS, PacketGpsSetIO, sizeof(PacketGpsSetIO));

            TSIP_SendPacket(TSIPTYPE_REQUEST_LAST_FIX, SuperPacketConfig , sizeof(SuperPacketConfig));
        }

        if ( result == 3 ) {
            // Signal horaire
            if (GpsHeure == 1) {

                if ( ConfigGPS == 0) {

                    // => configuration du GPS en simple precision + request last fix
                    TSIP_SendPacket(TSIPTYPE_SET_IO_OPTIONS, PacketGpsSetIO, sizeof(PacketGpsSetIO));

                    TSIP_SendPacket(TSIPTYPE_REQUEST_LAST_FIX, SuperPacketConfig , sizeof(SuperPacketConfig));

                    ConfigGPS = 1;
                }

                M10::digitalWrite( M10::LED, M10::LOW ) ;
                GpsHeure = 0;
            }
            else {

                M10::digitalWrite( M10::LED, M10::HIGH ) ;
                GpsHeure = 1;
            }
        }

        if ( result == 4 ) {
            // GPS en mode 3D
            M10::digitalWrite( M10::LED, M10::LOW );

            position.Lat = ptrGPpsInfos->PosLLA.lat.i;
            position.Lon = ptrGPpsInfos->PosLLA.lon.i;
            position.Alt = ptrGPpsInfos->PosLLA.altLong;

            date.Date =  ptrGPpsInfos->WeekNum;
            date.Time =  (uint32_t)ptrGPpsInfos->TimeOfWeek.f;
            date.UtcOffset = ptrGPpsInfos->UtcOffset.i;

            int numSVs = ptrGPpsInfos->GPSModeInfos.nSVs;

            speed.vE = ptrGPpsInfos->vE;
            speed.vN = ptrGPpsInfos->vN;
            speed.vU = ptrGPpsInfos->vU;

            // Turn transmitter on
            M10::synthPower( true ) ;

            // Configure ADF 7012
            radio.setup() ;

            // Turn on led when TXing
            M10::digitalWrite( M10::LED, M10::HIGH ) ;

            // Prepare packet
            M10Packet::preparePacket( &position, &speed, &date, numSVs, "F4AAA", 5, rawPacket, &rawPacketSize ) ;

            // Disable GPS RX interrupt
            IE2 &= ~UCA0RXIE;
            radio.send_data( rawPacket, rawPacketSize, 20 ) ;
            // Enable GPS RX interrupt
            IE2 |= UCA0RXIE;

            // TX has ended, turn off led
            M10::digitalWrite( M10::LED, M10::LOW ) ;

            // Turn transmitter off
            M10::synthPower( false ) ;

            bufferFlush( com_GetRxBuffer() );

            // Go to sleep, we will wake up when we receive a full GPS message
        }
#else
        if ( gps.decode() )
        {
            // Turn transmitter on
            M10::synthPower( true ) ;

            // Configure ADF 7012
            radio.setup() ;

            // Update packet with GPS data, continue if position is valid
            bool isValid = false ;
            Position position = gps.getPosition( &isValid ) ;
            if ( ! isValid )
            {
                // Break here if you do not want to send a message with invalid coordinates
                //break ;
            }

            Speed speed = gps.getSpeed() ;
            Datation date = gps.getTime() ;

            // Turn on led when TXing
            M10::digitalWrite( M10::LED, M10::HIGH ) ;

            // Prepare packet
            M10Packet::preparePacket( &position, &speed, &date, 6, "F4AAA", 5, rawPacket, &rawPacketSize ) ;

            // Disable GPS RX interrupt
            IE2 &= ~UCA0RXIE;
            radio.send_data( rawPacket, rawPacketSize, 20 ) ;
            // Enable GPS RX interrupt
            IE2 |= UCA0RXIE;

            // TX has ended, turn off led
            M10::digitalWrite( M10::LED, M10::LOW ) ;

            // Turn transmitter off
            M10::synthPower( false ) ;

            // Empty gps frame buffer
            bufferDumpFromFront(com_GetRxBuffer(), stdFLEN) ;
        }
#endif

        // Start sleeping
        // Enter LPM3, interrupts enabled
        __bis_SR_register( LPM3_bits + GIE );
    }
}

// Interrupt code for GPS RX
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  uint8_t charReceive = UCA0RXBUF;

  if ( bufferAddToEnd(&uartRxBuffer, charReceive) == 0 )
  {
      // no space in buffer
      // count overflow
      uartRxOverflow++;
  }

  // Clear LPM3 bits from 0(SR)
  __bic_SR_register_on_exit(LPM3_bits);

}

// Flag to handle power off sequence
bool isShutingDown = false ;

// Timer ISR
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void)
{
    // If the user is still pushing the switch
    if ( isShutingDown && ! ( P2IN & BIT0 ) )
    {
        // End end of life timer
        TACCR0 = 0 ;

        M10::digitalWrite( M10::LED, M10::HIGH ) ;

        // Flash led to indicate shutdown
        for ( unsigned char i = 6 ; i > 0 ; --i )
        {
            M10::toggleLed() ;
            M10::delay(100) ;
        }

        // These violent delights have violent ends
        // Machine commits power off
        M10::mainPower( false ) ;

        // It only has power until human finger releases switch
        // This will happen any time soon
        // Good bye
    }
    else
    {
        isShutingDown = false ;
    }

    // Clear LPM3 bits from 0(SR)
    __bic_SR_register_on_exit(LPM3_bits);
}

// Port 2 interrupt service routine (power button)
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    // Start end of life timer
    TACCR0 = 5000 ;

    // P2.0 IFG clear
    P2IFG &= (~BIT0) ;

    isShutingDown = true ;
}
