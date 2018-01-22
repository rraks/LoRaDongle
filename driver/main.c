#include <stdio.h>

#include "dongle.h"


int main(){

	const uint8_t *port_name = "/dev/ttyUSB0";

	init_driver(port_name);

	AuthenticateDongle();


//
//	uint8_t testData[16] = {0x01,0x1a,0x70,0xb3,0xd5,0x8f,0xf0,0x03,0x1d,0xe5,0xec,0x80,0xb5,0x50,0xe4,0x00};
//	uint8_t out[16];
//
//	PayloadEncrypt(testData, 15, dongleAESKey, 0, 0, 0, out);
//
//	// Example to write data.
//	int write = write_serial(fd, "1,0,250");
//
//	// Example for reading data.
//	unsigned char buf[80];
//	int read = read_serial(fd,buf);
//	printf("Data = %s\n", buf);
//
//
	return 0;

}
