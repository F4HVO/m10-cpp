/*
 * RobustPacket.h
 *
 *  Created on: 22 Jun 2020
 *      Author: hugo
 */

#pragma once

#include <stdint.h>

class RobustPacket
{
public:
    static void preparePacket( uint8_t buffer[], uint32_t bufferSize,
                               uint8_t outputData[], uint32_t * outputSize ) ;
};

