
#ifndef SERIAL_H
#define SERIAL_H


int set_interface_attribs(int fd, int speed);
void set_mincount(int fd, int mcount);
int open_serial(const uint8_t * portname);
int read_serial(int fd, unsigned char *buffer, int maxlen);
int write_serial(int fd, uint8_t *data, int datalen);


#endif
