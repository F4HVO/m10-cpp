/*
 * M10Packet.cpp
 *
 *  Created on: 27 Sep 2020
 *      Author: hugoc
 */

#include <M10Packet.h>



int update_checkM10(int c, unsigned short b) {
    int c0, c1, t, t6, t7, s;

    c1 = c & 0xFF;

    // B
    b = (b >> 1) | ((b & 1) << 7);
    b ^= (b >> 2) & 0xFF;

    // A1
    t6 = (c & 1) ^ ((c >> 2) & 1) ^ ((c >> 4) & 1);
    t7 = ((c >> 1) & 1) ^ ((c >> 3) & 1) ^ ((c >> 5) & 1);
    t = (c & 0x3F) | (t6 << 6) | (t7 << 7);

    // A2
    s = (c >> 7) & 0xFF;
    s ^= (s >> 2) & 0xFF;


    c0 = b ^ t ^ s;

    return ((c1 << 8) | c0) & 0xFFFF;
}

int getCRC( uint8_t * frame_bytes, int32_t frameLength )
{
    int i, cs;

    cs = 0;
    for (i = 0; i < frameLength-1; i++) {
        cs = update_checkM10(cs, frame_bytes[i]);
    }

    return cs ;
}

void writeMsb( int32_t val, uint8_t * dest )
{
    for ( int i = 0 ; i < 4 ; ++i )
    {
        *dest = ( val >> ( 8 * ( 3 - i ) ) ) & 0xFF ;
        dest++ ;
    }
}

void writeMsb3( int32_t val, uint8_t * dest )
{
    for ( int i = 0 ; i < 3 ; ++i )
    {
        *dest = ( val >> ( 8 * ( 2 - i ) ) ) & 0xFF ;
        dest++ ;
    }
}

void writeMsb2( int32_t val, uint8_t * dest )
{
    for ( int i = 0 ; i < 2 ; ++i )
    {
        *dest = ( val >> ( 8 * ( 1 - i ) ) ) & 0xFF ;
        dest++ ;
    }
}

void writeMsb( int64_t val, uint8_t * dest, int16_t size )
{
    for ( int i = 0 ; i < 5 ; ++i )
    {
        *dest = ( val >> ( 8 * ( size - 1 - i ) ) ) & 0xFF ;
        dest++ ;
    }
}

void
M10Packet::preparePacket( const Position * position,
                          const Speed * speed,
                          const Datation * date,
                          const char * sn,
                          int snSize,
                          uint8_t outputData[],
                          uint32_t * packetSize )
{
    *packetSize = 20 + 101 ;

    for ( int i = 150 ; i >=0 ; --i )
    {
        outputData[i] = 0 ;
    }

    // Add 0x99 before header
    for ( int i = 15 ; i >=0 ; --i )
    {
        outputData[i] = 0x99 ;
    }

    outputData += 16;

    // Add header 0x99 99 4C 99
    outputData[0] = 0x99 ;
    outputData[1] = 0x99 ;
    outputData[2] = 0x4C ;
    outputData[3] = 0x99 ;


    // Offsets are given after header
    outputData += 4;

    outputData[0] = 0x64 ;


#ifdef M10PLUS
    outputData[1] = 0xAF ;
    outputData[2] = 0x02 ;
    writeMsb( position->Lat, &outputData[0x4] ) ;
    writeMsb( position->Lon, &outputData[0x8] ) ;
    writeMsb3( position->Alt, &outputData[0xC] ) ;

    writeMsb2( speed->vE, &outputData[0xF] ) ;
    writeMsb2( speed->vN, &outputData[0x11] ) ;
    writeMsb2( speed->vU, &outputData[0x13] ) ;

    writeMsb3( date->Time, &outputData[0x15] ) ;
    writeMsb3( date->Date, &outputData[0x18] ) ;
#else
    outputData[1] = 0x9F ;
    outputData[2] = 0x20 ;
    uint64_t lat = position->Lat ;
    lat *= 0xB60b60 ;
    lat /= 1000000 ;
    uint64_t lon = position->Lon ;
    lon *= 0xB60b60 ;
    lon /= 1000000 ;

    // Number of satellites
    outputData[0x1E] = 0x05 ;
    writeMsb( lat, &outputData[0xE] ) ;
    writeMsb( lon, &outputData[0x12] ) ;
    writeMsb( position->Alt * 10, &outputData[0x16] ) ;


    writeMsb2( speed->vE, &outputData[0x4] ) ;
    writeMsb2( speed->vN, &outputData[0x6] ) ;
    writeMsb2( speed->vU, &outputData[0x8] ) ;
    writeMsb( date->Time*1000, &outputData[0x0A] ) ;
    writeMsb2( ( date->Date >> 8 ) + 2048, &outputData[0x20] ) ;
#endif

    for ( uint16_t i = 0 ; i < snSize ; ++i )
    {
        outputData[0x5D+i] = sn[i] ;
    }

    // Compute CRC
    uint16_t crc = getCRC( outputData, 0x64 ) ;
    outputData[0x63] = (crc >> 8) ;
    outputData[0x64] = (crc & 0x00FF) ;

    uint8_t lastBit = 1 ;
    static const uint8_t shift = 1 ;
    uint8_t newBit = lastBit ;

    // Differential encoding
    for ( int i = 0 ; i < *packetSize - 8 ; ++i )
    {
        for ( int j = 7 ; j >= 0 ; -- j )
        {
            uint8_t currentBit = ( outputData[i] >> j ) & 0x1 ;
            // On 0 change bit, on 1 keep last bit
            if ( currentBit == 0 )
            {
                newBit = ! lastBit ;
            }

            // Set new bit
            if ( newBit )
            {
                outputData[i] |= shift << j ;
            }
            else
            {
                // Unset new bit
                outputData[i] &= ~( shift << j ) ;
            }
            lastBit = newBit ;
        }
    }
}
