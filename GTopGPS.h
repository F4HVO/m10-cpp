/*
 * GTopGPS.h
 *
 *  Created on: 29 mai 2019
 *      Author: hugo
 */

#include <stdint.h>
#include "buffer.h"
#include "M10Data.h"

#pragma once


#define stdFLEN        0x3E  // 63 bytes
#define pos_GPSlat     0x05  // 4 byte
#define pos_GPSlon     0x09  // 4 byte
#define pos_GPSalt     0x0D  // 3 byte
#define pos_GPSvE      0x10  // 2 byte
#define pos_GPSvN      0x12  // 2 byte
#define pos_GPSvU      0x14  // 2 byte
#define pos_GPStime    0x16  // 3 byte
#define pos_GPSdate    0x19  // 3 byte
#define pos_Check      0x3D  // 2 byte



uint32_t get_4bytes(cBuffer * packet, unsigned int pos) ;

uint32_t get_3bytes(cBuffer * packet, unsigned int pos) ;

short get_2bytes(cBuffer * packet, unsigned int pos) ;

class GTopGPS
{
public :
    GTopGPS() ;

    void setBuffer( cBuffer * buffer ) ;

    bool decode() ;

    static bool checkM10(uint8_t *msg, int len) ;

    Position getPosition( bool * isValid ) ;

    Speed getSpeed() ;

    Datation getTime() ;

    void clear() ;

    // Format time as hhmmss
    void getTime( unsigned char string[] ) ;

    // Format position as ddmm.hhN/dddmm.hhE
    // Return trus if position is OK
    bool getPosition( unsigned char string[] ) ;


private :

    unsigned int index_ ;
    cBuffer * packet_ = nullptr ;

    uint8_t header_[3] ;
    unsigned int headerIndex_ ;

    bool headerFound_ ;
};

