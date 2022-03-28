/*! \file buffer.c \brief Multipurpose byte buffer structure and methods. */
//*****************************************************************************
//
// File Name	: 'buffer.c'
// Title		: Multipurpose byte buffer structure and methods
// Author		: Pascal Stang - Copyright (C) 2001-2002
// Created		: 9/23/2001
// Revised		: 9/23/2001
// Version		: 1.0
// Target MCU	: any
// Editor Tabs	: 4
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

/* https://github.com/synic/avrlib/blob/master/buffer.c */


#include "buffer.h"

#ifndef CRITICAL_SECTION_START
#define CRITICAL_SECTION_START  __disable_interrupt();
#define CRITICAL_SECTION_END    __enable_interrupt();
#endif

/************************** Fonctions ********************************************/

/** \defgroup Buffer_functions   Fonctions de gestion des buffers d'emission/reception RS232

  Fonctions de gestion des buffers d'emission/reception RS232
*/

/**********************************************************************/
/*! \brief		Fonction d'initialisation du module buffer
 *  \param		buffer	pointeur sur le buffer
 *  \param		start	pointeur sur la zone memoire a utilisee pour le buffer
 *  \param		size	taille du buffer
 */
/**********************************************************************/
void bufferInit(cBuffer* buffer, unsigned char *start, int size)
{
	// begin critical section
	CRITICAL_SECTION_START;
	// set start pointer of the buffer
	buffer->dataptr = start;
	buffer->size = size;
	// initialize index and length
	buffer->dataindex = 0;
	buffer->datalength = 0;
	// end critical section
	CRITICAL_SECTION_END;
}

/**********************************************************************/
/*! \brief		Fonction de depilage d'un caractere du buffer
 *  \param		buffer	pointeur sur le buffer
 */
/**********************************************************************/
unsigned char bufferGetFromFront(cBuffer* buffer)
{
	unsigned char donnee = 0;
	
	// begin critical section
	CRITICAL_SECTION_START;
	// check to see if there's data in the buffer
	if ( ( buffer->datalength > 0 ) && ( buffer->size > 0 ) )
	{
		// get the first character from buffer
		donnee = buffer->dataptr[buffer->dataindex];
	
		// move index down and decrement length
		buffer->dataindex++;
	
		if ( buffer->dataindex >= buffer->size )
		{
			buffer->dataindex %= buffer->size;
		}
		
		buffer->datalength--;
	}
	// end critical section
	CRITICAL_SECTION_END;
	// return
	return donnee;
}

/**********************************************************************/
/*! \brief		Fonction de lecture d'un ou de plusieurs caractere du buffer
 *  \param		buffer		pointeur sur le buffer
 *  \param		numbytes	nombre de caracteres a lire
 */
/**********************************************************************/
void bufferDumpFromFront(cBuffer* buffer, int numbytes)
{
	// begin critical section
	CRITICAL_SECTION_START;
	// dump numbytes from the front of the buffer
	// are we dumping less than the entire buffer?
	if ( ( numbytes < buffer->datalength ) && ( buffer->size > 0 ) )
	{
		// move index down by numbytes and decrement length by numbytes
		buffer->dataindex += numbytes;
		
		if ( buffer->dataindex >= buffer->size )
		{
			buffer->dataindex %= buffer->size;
		}

		buffer->datalength -= numbytes;
	}
	else
	{
		// flush the whole buffer
		buffer->datalength = 0;
	}
	// end critical section
	CRITICAL_SECTION_END;
}

/**********************************************************************/
/*! \brief		Fonction de lecture d'un caractere du buffer a un offset particulier
 *  \param		buffer		pointeur sur le buffer
 *  \param		index		offset de lecture
 *  \return		le caractere
 */
/**********************************************************************/
unsigned char bufferGetAtIndex(cBuffer* buffer, int index)
{
	unsigned char data = 0;

	// begin critical section
	CRITICAL_SECTION_START;

	if ( buffer->size > 0 )
	{
		// return character at index in buffer
		data = buffer->dataptr[(buffer->dataindex+index)%(buffer->size)];
	}
	// end critical section
	CRITICAL_SECTION_END;

	return data;
}

/**********************************************************************/
/*! \brief		Fonction d'empilage d'un caractere dans le buffer
 *  \param		buffer		pointeur sur le buffer
 *  \param		donnee		caractere à empiler
 *  \return		0 : Erreur , 1 : OK
 */
/**********************************************************************/
unsigned char bufferAddToEnd(cBuffer* buffer, unsigned char donnee)
{
    // begin critical section
    CRITICAL_SECTION_START;
    // make sure the buffer has room
    if ( ( buffer->datalength < buffer->size ) && ( buffer->size > 0 ) )
    {
        // save data byte at end of buffer
        buffer->dataptr[(buffer->dataindex + buffer->datalength) % buffer->size] = donnee;
        // increment the length
        buffer->datalength++;
        // end critical section
        CRITICAL_SECTION_END;
        // return success
        return 1;
    }
    // end critical section
    CRITICAL_SECTION_END;
    // return failure
    return 0;
}

/**********************************************************************/
/*! \brief		Fonction indiquant si le buffer est plein
 *  \param		buffer		pointeur sur le buffer
 *  \return		true = place disponible , false = plus de place disponible
 */
/**********************************************************************/
unsigned char bufferIsNotFull(cBuffer* buffer)
{
	// begin critical section
	CRITICAL_SECTION_START;
	// check to see if the buffer has room
	// return true if there is room
	unsigned short bytesleft = (buffer->size - buffer->datalength);
	// end critical section
	CRITICAL_SECTION_END;
	return bytesleft;
}

/**********************************************************************/
/*! \brief		Fonction permettant d'effacer le buffer
 *  \param		buffer		pointeur sur le buffer
 */
/**********************************************************************/
void bufferFlush(cBuffer* buffer)
{
	// begin critical section
	CRITICAL_SECTION_START;
	// flush contents of the buffer
	buffer->dataindex = 0;
	buffer->datalength = 0;
	// end critical section
	CRITICAL_SECTION_END;
}

/**********************************************************************/
/*! \brief		Fonction indiquant le nombre de caracteres du buffer
 *  \param		buffer		pointeur sur le buffer
 *  \return		nombre de caracteres presents dans le buffer
 */
/**********************************************************************/
int bufferLength(cBuffer* buffer)
{
    int lg;

    // begin critical section
    CRITICAL_SECTION_START;
	lg = (int)buffer->datalength;
    // end critical section
    CRITICAL_SECTION_END;

    return lg;
}