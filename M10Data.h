/*
 * M10Data.h
 *
 *  Created on: 28 Mar 2022
 *      Author: hugoc
 */

#pragma once


#include <stdint.h>

struct Position {
    int32_t Lat;
    int32_t Lon;
    int32_t Alt;
};

struct Speed {
    int vE;
    int vN;
    int vU;
    int Cap;
};

struct Datation {
    uint32_t Date;
    uint32_t Time;
    uint32_t UtcOffset;
};
