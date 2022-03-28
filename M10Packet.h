/*
 * M10Packet.h
 *
 *  Created on: 27 Sep 2020
 *      Author: hugoc
 */

#pragma once

#include "M10Data.h"


class M10Packet
{
public:
    static void preparePacket( const Position * position,
                               const Speed * speed,
                               const Datation * date,
                               int numSV,
                               const char * sn,
                               int snSize,
                               uint8_t outputData[],
                               uint32_t * packetSize ) ;
};

