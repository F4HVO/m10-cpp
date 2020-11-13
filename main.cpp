/*
 * main.cpp
 *
 *  Created on: 27 Apr 2020
 *      Author: hugo
 */

#include <msp430.h>
#include "M10Configuration.h"
#include "RadioAdf7012.h"
#include "GTopGPS.h"
#include "M10Packet.h"
#include <string.h>


// Gps frame decoder
GTopGPS gps ;

volatile bool newGpsPosition = false ;

// Primitive machine state
// Nominal mode : start in SENDING_POS state
// Test mode : start in TEST_MODE
enum { SENDING_POS,
       TEST_MODE } beaconState = SENDING_POS ;

// We will send several positions every time the beacon enters SENDING_POS state
int positionCounter = 0 ;

// We will sleep several "cycles"
unsigned char sleepCycles = 1 ;

// Binary packet to send over the air
unsigned char rawPacket[250] ;
uint32_t rawPacketSize = 0 ;

// Handle radio configuration and operation
RadioAdf7012 radio ;

int main()
{
    // Setup board
    M10::setup() ;

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
    if ( beaconState != TEST_MODE )
    {
        M10::gpsPower( true ) ;
    }

    __bis_SR_register( GIE ) ;

    TACCR0 = 0;

    // Main loop
    while ( true )
    {
        switch ( beaconState )
        {
            // In this state, we wait for good GPS data, GPS UART keeps waking us up
            case SENDING_POS:
            {
                // Turn transmitter on
                M10::synthPower( true ) ;

                // Configure ADF 7012
                radio.setup() ;

                // If new GPS available, TX updated position
                if ( ! newGpsPosition )
                {
                    break ;
                }

                // A packet has been decoded, flash led
                newGpsPosition = false ;

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
                M10Packet::preparePacket( &position, &speed, &date, "F4AAA", 5, rawPacket, &rawPacketSize ) ;

                // Disable GPS RX interrupt
                IE2 &= ~UCA0RXIE;
                radio.send_data( rawPacket, rawPacketSize, 20 ) ;
                // Enable GPS RX interrupt
                IE2 |= UCA0RXIE;

                // TX has ended, turn off led
                M10::digitalWrite( M10::LED, M10::LOW ) ;

                // Turn transmitter off
                M10::synthPower( false ) ;



                // Go to sleep, we will wake up when we receive a full GPS message
                break ;
            }

            // Send continuously
            case TEST_MODE:
            {
                // Turn transmitter on
                M10::synthPower( true ) ;

                // Configure ADF 7012
                radio.setup() ;

                // Turn on led when TXing
                M10::digitalWrite( M10::LED, M10::HIGH ) ;

                Position position ;
                position.Lat = 42000000 ;
                position.Lon = 1000000 ;
                position.Alt = 12000 ;

                Speed speed ;
                speed.vE = 10 ;
                speed.vN = 0 ;
                speed.vU = 12 ;

                Datation date ;
                date.Date = 10000 ;
                date.Time = 100226 ;

                // Prepare raw packet
                M10Packet::preparePacket( &position, &speed, &date, "Test", 4, rawPacket, &rawPacketSize ) ;

                // Send packet over the air
                radio.send_data( rawPacket, rawPacketSize, 20 ) ;

                M10::digitalWrite( M10::LED, M10::LOW ) ;

                // Timer to wake up in the future
                TACCR0 = 2000 ;
            }
        }

        // Start sleeping
        // Enter LPM3, interrupts enabled
        __bis_SR_register( LPM3_bits + GIE );
    }
}

// Interrupt code for GPS RX
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{

  char c = UCA0RXBUF ;
  // If we received a full GPS message
  if ( gps.encode( c ) )
  {
      newGpsPosition = true ;

      // Reset parsing state (value stays unchanged)
      gps.clear() ;

      // Clear LPM3 bits from 0(SR)
      __bic_SR_register_on_exit(LPM3_bits);
  }
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
