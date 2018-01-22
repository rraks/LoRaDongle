
#ifndef DONGLE_H_
#define DONGLE_H_


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>






/*
 *  Init Driver
 *
 */


void init_driver(const char *port_name);


/*
 *  Functions to encrypt and decrypt according to Brian Gladman's open source AES implementation
 *
 */
void PayloadEncrypt( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *encBuffer );

void PayloadDecrypt( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *decBuffer );


/*
 *  Function to perform CMAC by Lander Casado, Philippas Tsigas
 *
 */

void ComputeMic( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint32_t *mic );


/*
 *
 *  Function that performs the Authentication step
 *
 */

void AuthenticateDongle();

uint8_t shuffleStr(uint8_t *dp, uint16_t length);




#endif /* DONGLE_H_ */
