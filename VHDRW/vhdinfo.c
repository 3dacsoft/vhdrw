#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "bswap.h"
#include "vhd.h"

int main(int argc, char *argv[])
{
	FILE *vhd;
	long filesize;
	void *tempptr;
	char footerstring[17];
	uint64_t footerqword;
	uint32_t footerdword;
	uint16_t footerword;
	uint8_t footerbyte;
	int count;
	char metric[5][3] = {"B", "KB", "MB", "GB", "TB"};
	uint32_t checksum;
	char footerbuf[512];
	
	
	//Check for valid arguments
	if(argc != 2) { printf("wrong number of arguements (%d)\nArguments:\n", argc); for(count=0;count<argc;count++) printf("%d: %s\n", count+1, argv[count]); return 1;}
	vhd = fopen(argv[1], "rb");
	//vhd = fopen("c:\\vm\\ostest.vhd", "rb");
	if(!vhd) { printf("File open failed!\n"); return 1; }

	//Arguments good, now look for footer at last 512 bytes
	fseek(vhd, -(FOOTER_SIZE), SEEK_END);	//seek to end and read back 512 bytes
	fread(footerbuf, 1, FOOTER_SIZE, vhd);
	fclose(vhd);

	//Validate checksum before reading info
	checksum = 0;
	//total bytes from footer without checksum itself
	for(count = 0; count < FOOTER_SIZE; count++)
		if(count < FOOTER_CHECKSUM_OFFSET || count > FOOTER_CHECKSUM_LASTBYTE) checksum += *(uint8_t*)(footerbuf+count);

	footerdword = bswap_32(*(uint32_t*)(footerbuf + FOOTER_CHECKSUM_OFFSET)); //read checksum, swap ends, and store in buffer

	if(validateVHDFooterChecksum(footerbuf)) printf("Footer checksum is valid.\n");
	else
	{
		printf("Footer checksum invalid!");
		return 1;
	}

	printf("VHD info:\n");
	
	//COOKIE
	//Copy cookie string to buffer then terminate
	strncpy(footerstring, footerbuf, COOKIE_SIZE);
	footerstring[COOKIE_SIZE] = 0;
	printf("Cookie: %s\n", footerstring);
	
	//FEATURES
	//Offset pointer, cast to 32-bit int ptr, deference, and finally byteswap to change endianness
	footerdword = bswap_32(*(uint32_t*)(footerbuf + FEATURES_OFFSET));
	printf("Features: ");
	if(!footerdword ^ FEATURE_RESERVED) printf("None\n");
	else
	{
		if(footerdword & 1) printf("Temporary ");
		printf("\n");
	}
	
	//VHD FILE FORMAT VERSION
	printf("Format version: ");
	footerword = bswap_16(*(uint16_t*)(footerbuf + FORMAT_VERSION_MAJOR_OFFSET));
	printf("%d.", footerword);
	footerword = bswap_16(*(uint16_t*)(footerbuf + FORMAT_VERSION_MINOR_OFFSET));
	printf("%d\n", footerword);
	
	//DATA OFFSET
	footerqword = bswap_64(*(uint64_t*)(footerbuf + FOOTER_DATA_OFFSET_OFFSET));
	printf("Data offset: 0x%X ", footerqword);
	printf("(%d) bytes\n", footerqword);
	
	//TIME STAMP
	/*footerdword = bswap_32(*(uint32_t*)(footerbuf + TIME_STAMP_OFFSET));
	printf("Time stamp seconds: %d\n", footerdword);
	footerword = footerdword/31536000;
	printf("Year: %d\n", 2000+ footerword);
	footerword = fooderdword - (footerword * 365);
	printf("Month: %d\n*/
	
	//CREATOR APPLICATION
	strncpy(footerstring, footerbuf + CREATOR_APP_OFFSET, CREATOR_APP_SIZE);
	footerstring[CREATOR_APP_SIZE] = 0;
	tempptr = strchr(footerstring, ' ');
	if(tempptr) footerstring[(char*)tempptr-footerstring] = 0;
	printf("Creator application: %s ", footerstring);
	footerword = bswap_16(*(uint16_t*)(footerbuf + CREATOR_VERSION_MAJOR_OFFSET));
	printf("v%d.", footerword);
	footerword = bswap_16(*(uint16_t*)(footerbuf + CREATOR_VERSION_MINOR_OFFSET));
	printf("%d", footerword);
	strncpy(footerstring, footerbuf + CREATOR_OS_OFFSET, CREATOR_OS_SIZE);
	footerstring[CREATOR_OS_SIZE] = 0;
	if(!strcmp(footerstring, CREATOR_OS_WINDOWS)) printf(" on Windows");
	if(!strcmp(footerstring, CREATOR_OS_MAC)) printf(" on MacOS");
	printf("\n");
	
	//ORIGINAL SIZE
	footerqword = bswap_64(*(uint64_t*)(footerbuf + ORIGINAL_SIZE_OFFSET));
	count = 0;
	while(footerqword > 0x400)
	{
		footerqword /= 0x400;
		count ++;
	}
	printf("Original size: %d ", footerqword);
	printf("%s\n", metric[count]);
	
	//CURRENT SIZE
	footerqword = bswap_64(*(uint64_t*)(footerbuf + CURRENT_SIZE_OFFSET));
	count = 0;
	while(footerqword > 0x400)
	{
		footerqword /= 0x400;
		count ++;
	}
	printf("Current size: %d ", footerqword);
	printf("%s\n", metric[count]);
	
	//DISK GEOMETRY
	footerword = bswap_16(*(uint16_t*)(footerbuf + GEOMETRY_CYLINDER_OFFSET));
	printf("Disk geometry: %d Cylinders, ", footerword);
	footerbyte = *(uint8_t*)(footerbuf + GEOMETRY_HEAD_OFFSET);
	printf("%d Heads, ", footerbyte);
	footerbyte = *(uint8_t*)(footerbuf + GEOMETRY_SECTOR_OFFSET);
	printf("%d Sectors\n", footerbyte);
	
	//DISK TYPE
	footerdword = bswap_32(*(uint32_t*)(footerbuf + DISK_TYPE_OFFSET));
	switch(footerdword)
	{
		case 1:
			strcpy(footerstring, "Reserved");
			break;
		case 2:
			strcpy(footerstring, "Fixed");
			break;
		case 3:
			strcpy(footerstring, "Dynamic");
			break;
		case 4:
			strcpy(footerstring, "Differencing");
			break;
		default:
			strcpy(footerstring, "Invalid");
	}
	printf("Disk type: %s\n", footerstring);
		
	return 0;
}