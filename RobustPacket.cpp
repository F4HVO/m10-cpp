/*
 * RobustPacket.cpp
 *
 *  Created on: 22 Jun 2020
 *      Author: hugo
 */

#include <RobustPacket.h>

uint16_t CRC16(unsigned char *buf, int len)
{
  unsigned int crc = 0xFFFF;
  for (unsigned int pos = 0; pos < len; pos++)
  {
  crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

  for (int i = 8; i != 0; i--) {    // Loop over each bit
    if ((crc & 0x0001) != 0) {      // If the LSB is set
      crc >>= 1;                    // Shift right and XOR 0xA001
      crc ^= 0xA001;
    }
    else                            // Else LSB is not set
      crc >>= 1;                    // Just shift right
    }
  }

  return crc;
}


void encode(uint8_t *input,
            uint32_t inputSize,
            uint8_t *polynomials,
            uint32_t polynomialSize,
            uint8_t *output,
            uint32_t *outputSize)
{
    uint8_t state = 0;
    uint8_t outputBitCounter = 0;
    uint8_t outputCurrentWord = 0;
    *outputSize = 0;
    // For each input 8bit word
    for (uint32_t i = 0; i < inputSize; ++i)
    {
        uint8_t currentWord = input[i];
        // For each bit of word
        for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex)
        {
            // Start with input msb
            // Insert bit into state
            state <<= 1;
            state |= ((currentWord & 0x80) >> 7);
            currentWord <<= 1;

            // Compute polynomials and output bits
            for (int polIndex = 0; polIndex < polynomialSize; ++polIndex)
            {
                uint8_t product = polynomials[polIndex] & state;
                uint8_t sum = 0;
                for (uint8_t b = 0; b < 8; ++b)
                {
                    sum += (product & (1 << b)) > 0 ? 1 : 0;
                }

                outputCurrentWord <<= 1;
                outputCurrentWord |= (sum & 0x1);
                if (++outputBitCounter == 8)
                {
                    output[*outputSize] = outputCurrentWord;
                    outputCurrentWord = 0;
                    *outputSize += 1;
                    outputBitCounter = 0;
                }
            }
        }
    }

    // Flush a complete word (only 7 bits needed by we manipulate 8bit words so we flush one word)
    for (uint8_t flushIndex = 0; flushIndex < 8; ++flushIndex)
    {
        state <<= 1;

        // Compute polynomials and output bits
        for (int polIndex = 0; polIndex < polynomialSize; ++polIndex)
        {
            uint8_t product = polynomials[polIndex] & state;
            uint8_t sum = 0;
            for (uint8_t b = 0; b < 8; ++b)
            {
                sum += (product & (1 << b)) > 0 ? 1 : 0;
            }

            outputCurrentWord <<= 1;
            outputCurrentWord |= (sum & 0x1);
            if (++outputBitCounter == 8)
            {
                output[*outputSize] = outputCurrentWord;
                outputCurrentWord = 0;
                *outputSize += 1;
                outputBitCounter = 0;
            }
        }
    }
}

void
RobustPacket::preparePacket( uint8_t buffer[], uint32_t bufferSize,
                             uint8_t outputData[], uint32_t * outputSize )
{
    // Copy preamble
    for ( int i = 0 ; i < 5 ; ++i )
    {
        outputData[i] = buffer[i] ;
    }
    // Compute CRC
    uint16_t crc = CRC16( buffer + 5, bufferSize - 5 - 4 ) ;
    buffer[bufferSize-4] = (crc >> 8) ;
    buffer[bufferSize-3] = (crc & 0x00FF) ;
    uint8_t polynomials[] = {0xAA, 0x99} ;

    // Convolutional encoding over packet data
    encode(buffer+5, bufferSize-5, polynomials, 2, outputData+5, outputSize) ;
    outputSize += 5 ;
}
