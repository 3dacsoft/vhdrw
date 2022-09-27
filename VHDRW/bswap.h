/*
MACROS TO BTYE SWAP FOR ENDIANNESS CHANGE
*/

#ifndef BSWAP
#define BSWAP

#define bswap_32(x) ((x>>24)&0xff) | ((x<<8)&0xff0000) | ((x>>8)&0xff00) | ((x<<24)&0xff000000)
#define bswap_64(x) ((x>>56)&0xff) | ((x>>40)&0xff00) | ((x>>24)&0xff0000) | ((x>>8)&0xff000000) | ((x<<8)&0xff00000000) | \
					((x<<24)&0xff0000000000) | ((x<<40)&0xff000000000000) | ((x<<56)&0xff00000000000000)
#define bswap_16(x) (x >> 8 | x << 8)

#endif
