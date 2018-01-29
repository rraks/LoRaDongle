#include "dongle.h"
#include <stdlib.h>
#include <stdint.h>
#include "crypt/aes.h"
#include "serial/serial.h"
#include "crypt/cmac.h"
#include "string.h"
#include "base64/base64.h"
#include "config.h"
#include <time.h>


uint8_t isCommIfaceSet = 0;
uint8_t sessionActive = 0;

/*!
 * Encryption aBlock and sBlock
 */
static uint8_t aBlock[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                          };
static uint8_t sBlock[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                          };

/*!
 * AES computation context variable
 */
static aes_context AesContext;





/*!
 * CMAC/AES Message Integrity Code (MIC) Block B0 size
 */
#define DONGLE_MIC_BLOCK_B0_SIZE                   16

/*!
 * MIC field computation initial data
 */
static uint8_t MicBlockB0[] = { 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                              };

/*!
 * Contains the computed MIC field.
 *
 * \remark Only the 4 first bytes are used
 */
static uint8_t Mic[16];


/*!
 * CMAC computation context variable
 */
static AES_CMAC_CTX AesCmacCtx[1];



/*
 *  File descriptor for UART dongle
 */

int fd;


void init_driver(const char *port_name){


	fd = open_serial(port_name);
	if(fd != 0){
		isCommIfaceSet = 1;
	}


}



void PayloadEncrypt( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *encBuffer )
{
    uint16_t i;
    uint8_t bufferIndex = 0;
    uint16_t ctr = 1;

    memset( AesContext.ksch, '\0', 240 );
    aes_set_key( key, 16, &AesContext );

    aBlock[5] = dir;

    aBlock[6] = ( address ) & 0xFF;
    aBlock[7] = ( address >> 8 ) & 0xFF;
    aBlock[8] = ( address >> 16 ) & 0xFF;
    aBlock[9] = ( address >> 24 ) & 0xFF;

    aBlock[10] = ( sequenceCounter ) & 0xFF;
    aBlock[11] = ( sequenceCounter >> 8 ) & 0xFF;
    aBlock[12] = ( sequenceCounter >> 16 ) & 0xFF;
    aBlock[13] = ( sequenceCounter >> 24 ) & 0xFF;

    while( size >= 16 )
    {
        aBlock[15] = ( ( ctr ) & 0xFF );
        ctr++;
        aes_encrypt( aBlock, sBlock, &AesContext );
        for( i = 0; i < 16; i++ )
        {
            encBuffer[bufferIndex + i] = buffer[bufferIndex + i] ^ sBlock[i];
        }
        size -= 16;
        bufferIndex += 16;
    }

    if( size > 0 )
    {
        aBlock[15] = ( ( ctr ) & 0xFF );
        aes_encrypt( aBlock, sBlock, &AesContext );
        for( i = 0; i < size; i++ )
        {
            encBuffer[bufferIndex + i] = buffer[bufferIndex + i] ^ sBlock[i];
        }
    }
}

void PayloadDecrypt( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *decBuffer )
{
    PayloadEncrypt( buffer, size, key, address, dir, sequenceCounter, decBuffer );
}




/*!
 * \brief Computes the dongle frame MIC field
 *
 * \param [IN]  buffer          Data buffer
 * \param [IN]  size            Data buffer size
 * \param [IN]  key             AES key to be used
 * \param [IN]  address         Frame address
 * \param [IN]  dir             Frame direction [0: uplink, 1: downlink]
 * \param [IN]  sequenceCounter Frame sequence counter
 * \param [OUT] mic Computed MIC field
 */
void ComputeMic( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint32_t *mic )
{
    MicBlockB0[5] = dir;

    MicBlockB0[6] = ( address ) & 0xFF;
    MicBlockB0[7] = ( address >> 8 ) & 0xFF;
    MicBlockB0[8] = ( address >> 16 ) & 0xFF;
    MicBlockB0[9] = ( address >> 24 ) & 0xFF;

    MicBlockB0[10] = ( sequenceCounter ) & 0xFF;
    MicBlockB0[11] = ( sequenceCounter >> 8 ) & 0xFF;
    MicBlockB0[12] = ( sequenceCounter >> 16 ) & 0xFF;
    MicBlockB0[13] = ( sequenceCounter >> 24 ) & 0xFF;

    MicBlockB0[15] = size & 0xFF;

    AES_CMAC_Init( AesCmacCtx );

    AES_CMAC_SetKey( AesCmacCtx, key );

    AES_CMAC_Update( AesCmacCtx, MicBlockB0, DONGLE_MIC_BLOCK_B0_SIZE );

    AES_CMAC_Update( AesCmacCtx, buffer, size & 0xFF );

    AES_CMAC_Final( Mic, AesCmacCtx );

    *mic = ( uint32_t )( ( uint32_t )Mic[3] << 24 | ( uint32_t )Mic[2] << 16 | ( uint32_t )Mic[1] << 8 | ( uint32_t )Mic[0] );
}




void AuthenticateDongle()
{

	uint8_t authString[15];// = malloc(15*sizeof(uint8_t));
	uint8_t encAuthString[19];
	uint8_t b64encAuthString[25]; // = malloc(25*sizeof(uint8_t));
	uint8_t *recB64encAuthString = malloc(25*sizeof(uint8_t));



	uint32_t mic = 0;
	uint32_t b64len = 0;

	uint32_t micRx = 0;
	uint8_t serlen = 0;
	uint8_t shflidx = 0;
	int write=0;




	memcpy(&authString[1], dongleID, 8*sizeof(uint8_t));
	memcpy((&authString[1])+8*sizeof(uint8_t), gatewayID, 6*sizeof(uint8_t));

	shflidx = shuffleStr(authString,14);
	memcpy(authString,&shflidx, 1*sizeof(uint8_t));

    PayloadEncrypt( authString, 15, dongleKey, 0, 0, 0, encAuthString);
    ComputeMic(encAuthString, 15, dongleKey, 0, 0, 0, &mic);

    encAuthString[15] = mic & 0xFF;
    encAuthString[16] = ( mic >> 8 ) & 0xFF;
    encAuthString[17] = ( mic >> 16 ) & 0xFF;
    encAuthString[18] = ( mic >> 24 ) & 0xFF;

    b64len = b64_encode(encAuthString, 19, b64encAuthString);


    if(isCommIfaceSet == 1){

    	write = write_serial(fd, b64encAuthString, b64len);
    	write = write_serial(fd, "\n", 1);

    	memset(encAuthString, 0, 18*sizeof(uint8_t));
    	memset(b64encAuthString, 0, 25*sizeof(uint8_t));
    	memset(authString, 0, 15*sizeof(uint8_t));

    	serlen = read_serial(fd, b64encAuthString, 25); // Blocking read Ack from dongle

    	b64len = b64_decode(b64encAuthString, serlen-1, encAuthString);
    	PayloadDecrypt( encAuthString, 15, dongleKey, 0, 0, 0, authString);

		ComputeMic(encAuthString, b64len-4, dongleKey, 0, 0, 0, &mic);
		micRx |= ( uint32_t )encAuthString[b64len-3];
		micRx |= ( ( uint32_t )encAuthString[b64len-2] << 8 );
		micRx |= ( ( uint32_t )encAuthString[b64len-1] << 16 );
		micRx |= ( ( uint32_t )encAuthString[b64len] << 24 );

		if( mic == micRx ){
			sessionActive = 1;
		}


    }


	//int write = write_serial(fd, (char)encAuthString, 15);


	// UART Send

	// UART receive and check if MAC id is present

	// If present, flag = 1

}


uint8_t shuffleStr(uint8_t *dp, uint16_t length){

	uint8_t *tmpBuf = malloc(length*sizeof(uint8_t));
	uint16_t i = 0,j=0;
	uint8_t shuffleIndex;
	uint8_t tempVal=0;

	srand(time(NULL));
	shuffleIndex = rand()%length+1;


	for(j=0;j<length; j++){
		tmpBuf[(j+shuffleIndex)%length] = dp[j]; //0 is last, 1 is 0, 2 is 1
	}


	memcpy(dp, tmpBuf, length);
	free(tmpBuf);
	return shuffleIndex;

}
