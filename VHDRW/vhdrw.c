#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vhd.h"

#define MAX_ARGUMENTS 5
#define MIN_ARGUMENTS 5

int main(int argc, char *argv[])
{
	int count;
	FILE *VHDFile;
	FILE *BINFile;
	char *VHDPath;	   //argv[1]
	unsigned long operation; //argv[2]
	unsigned long sectorNumber;   //argv[3]
	char *BINPath;     //argv[4]
	uint32_t checksum;
	uint64_t headerOffset;
	uint64_t tableOffset;
	uint32_t blockSize;
	uint32_t maxTableEntries;
	uint32_t sectorsPerBlock;
	uint32_t blockNumber;
	uint32_t blockOffset;
	uint32_t BATEntry;
	uint32_t sectorOffset;
	uint32_t sectorInBlock;
	uint32_t bitmapSize;
	uint8_t bitmapByte;
	uint8_t sectorWritten;
	uint8_t footer[FOOTER_SIZE];
	uint8_t header[HEADER_SIZE];
	uint8_t sector[SECTOR_SIZE];
	int VHDType;
	
	//Check for correct number of arguments
	if(argc < MIN_ARGUMENTS || argc > MAX_ARGUMENTS)
	{
		printf("Wrong number of arguments (%d)\nArguments:\n", argc);
		for(count = 0; count < argc; count++) printf("%d: %s\n", count+1, argv[count]);
		printf("Correct usage: vhdrw <vhdpath> <r/w> <sector number> <binpath>");
		return 1;
	}
	
	VHDPath = argv[1];						//location of VHD file to work with
	operation = argv[2][0];					//'r' = read, 'w' = write, maybe more in future
	sectorNumber = strtoul(argv[3], NULL, 10);	//converts sector argument to unsigned long
	BINPath = argv[4];						//file which data is to be read from/written to
	
#ifdef _VHDDEBUG
	printf("You want to ");
	switch(operation)
	{
		case 'r':
			printf("read sector %d from %s to %s.\n", sectorNumber, VHDPath, BINPath);
			break;
		case 'w':
			printf("write sector %d to %s from %s.\n", sectorNumber, VHDPath, BINPath);
			break;
		default:
			printf("do something this program doesn't do. Bye.");
			return 1;
	}
#endif	
	VHDFile = fopen(VHDPath, "rb");			//Open VHD for reading first to collect file info
	if(!VHDFile) { printf("Could not open file\"%s\"!\n", VHDPath); return 1; }
	
	//This ugly statement attempts to seek to 512 (FOOTER_SIZE) bytes from the end of the file
	if(fseek(VHDFile, -FOOTER_SIZE, SEEK_END)) { printf("Error seeking to footer"); return 1; }
	//Read those last 512 bytes into buffer "footer" and use checksum to hold return value
	if(checksum = fread(footer, 1, FOOTER_SIZE, VHDFile) < FOOTER_SIZE) { printf("Error reading footer, only got %d byte(s)", checksum); fclose(VHDFile); return 1; }
	//pass footer into validation function
	if(!validateVHDFooterChecksum(footer)) { printf("Bad footer checksum\n"); return 1; }
	
	/*Determine VHD type (fixed/dynamic/differencing) from footer.
	  Currently only coding dynamic since that's what my test vhd is, fixed is easy and will come later */
	VHDType = getVHDType(footer);
	switch(VHDType)
	{
		case DISK_TYPE_FIXED:
			printf("VHD type is fixed disk, not implemented yet\n");
			return 1;
			//break;
		case DISK_TYPE_DYNAMIC:
			printf("VHD type is dynamic\n");
			headerOffset = getVHDHeaderOffset(footer);		//retrieves header offset from footer
			//Seek to header offset from beginning of file
			if(fseek(VHDFile, headerOffset, SEEK_SET)) { printf("Error seeking to header"); return 1; }
			//read 1024 (HEADER_SIZE) to 'header' array. Reusing checksum variable for read return value to print to error
			if(checksum = fread(header, 1, HEADER_SIZE, VHDFile) < HEADER_SIZE) { printf("Error reading header, only got %d bytes", checksum); return 1; }
			//validates dynamic disk header against checksum(and makes sure I'm reading it correctly) by passing pointer to header
			if(checksum = validateVHDHeaderChecksum(header)) {printf("Bad header checksum\n"); return 1;}
			
			//retrieve block allocation table location and block size from header
			tableOffset = getVHDTableOffset(header);
			blockSize = getVHDBlockSize(header);
			maxTableEntries = getVHDMaxTableEntries(header);

#ifdef _VHDDEBUG
			printf("tableOffset=0x%x\n", tableOffset);
			printf("blockSize=0x%x\n", blockSize);
			printf("maxTableEntries=%d\n", maxTableEntries);
#endif

			sectorsPerBlock = blockSize / SECTOR_SIZE;
			blockNumber = sectorNumber / sectorsPerBlock;

#ifdef _VHDDEBUG
			printf("%d sectors per block\n", sectorsPerBlock);
			printf("Sector %d is in block number %d\n", sectorNumber, blockNumber);
#endif

			//Seek to table entry offset in file
			if(fseek(VHDFile, tableOffset + (blockNumber * BAT_ENTRY_SIZE), SEEK_SET)) { printf("Error seeking to BAT entry"); return 1; }
			
			if(checksum = fread(&BATEntry, 1, BAT_ENTRY_SIZE, VHDFile) < BAT_ENTRY_SIZE) { printf("Error reading BAT entry, only got %d bytes", checksum); return 1; }
#ifdef _VHDDEBUG
			printf("Got 0x%x from BAT for block %d\n", bswap_32(BATEntry), blockNumber);
#endif			
			//change endianness because it would not work mid calcuation
			BATEntry = bswap_32(BATEntry);
			
			//Check to see if block has been created, can return 0 for read but cannot write to it without creating first
			if(BATEntry == 0xFFFFFFFF)
			{
				printf("Block %d has been been created yet\n", blockNumber);
				return 1;
			}
			else
			{
				blockOffset = BATEntry * SECTOR_SIZE;
				sectorInBlock = sectorNumber % sectorsPerBlock;
				bitmapSize = sectorsPerBlock / 8;
				if(sectorsPerBlock % SECTOR_SIZE > 1) bitmapSize += SECTOR_SIZE;
				sectorOffset = blockOffset + bitmapSize + (sectorInBlock * SECTOR_SIZE);
#ifdef _VHDDEBUG
				printf("sectorsPerBlock=%d\n", sectorsPerBlock);
				printf("Block offset is 0x%x\n", blockOffset);
				printf("bitmapSize=0x%x\n", bitmapSize);
				printf("sectorInBlock=%d\n", sectorInBlock);
				printf("sectorNumber=%d\n", sectorNumber);
#endif				
				//Seek to block bitmap sector where desired sector bit is
				if(fseek(VHDFile, blockOffset + (sectorInBlock / SECTOR_SIZE), SEEK_SET)) { printf("Error seeking to BAT entry"); fclose(VHDFile); return 1; }
				if(checksum = fread(sector, 1, SECTOR_SIZE, VHDFile) < SECTOR_SIZE) { printf("Error reading block bitmap, only got %d bytes", checksum); fclose(VHDFile); return 1; }
				fclose(VHDFile);	//Done getting necessary info from VHD File. Close to perform r/w operations

				//Save this particular byte from bitmap in case needed to change later
				bitmapByte = sector[sectorInBlock / 8];
				//Shift and bitwise and to retain only necessary flag for determining necessary actions later
				sectorWritten = (bitmapByte >> (7 - (sectorInBlock % 8))) & 1;
				
#ifdef _VHDDEBUG
				printf("bitflags %b\n", bitmapByte);
				fputs("sector written=", stdout);
				if(sectorWritten) puts("true");
				else puts("false");
#endif
			
				switch (operation)
				{
					case 'r':
						//Open VHD for reading first to collect file info
						if(!(VHDFile = fopen(VHDPath, "rb"))) { printf("Could not open file\"%s\"!\n", VHDPath); return 1; }

						if(!sectorWritten)
						{
							//Sector bit is 0 so assume zeroes and fill sector array
							puts("Sector never written. Returning zeros");
							for(count = 0; count < SECTOR_SIZE; count++) sector[count] = 0;
						}
						else
						{
							//Read sector from VHD file to sector[]
							if(fseek(VHDFile, sectorOffset, SEEK_SET)) { printf("Error seeking to BAT entry"); fclose(VHDFile); return 1; }
							if(checksum = fread(sector, 1, SECTOR_SIZE, VHDFile) < SECTOR_SIZE) { printf("Error reading block bitmap, only got %d bytes", checksum); fclose(VHDFile); return 1; }
						}
#ifdef _VHDDEBUG
						printf("Sector output from offset 0x%x: ", sectorOffset);
						for(count = 0; count < SECTOR_SIZE; count++) printf("%0x ", sector[count]);
						putchar('\n');
#endif
						fclose(VHDFile);
						//Write to bin file
						if(!(BINFile = fopen(BINPath, "wb"))) { printf("Error opening bin file for writing"); return 1;}
						if(checksum = fwrite(sector, 1, SECTOR_SIZE, BINFile) < SECTOR_SIZE) { printf("Error writing sector to bin file, only sent %d bytes", checksum); fclose(BINFile); return 1;}
						fclose(BINFile);
						printf("BIN file write appears to have been successful!\n");
						break;
					case 'w':
						//Read bin file into sector[]
						if(!(BINFile = fopen(BINPath, "rb"))) { printf("Could not open file\"%s\"!\n", BINPath); return 1; }
						if(checksum = fread(sector, 1, SECTOR_SIZE, BINFile) < SECTOR_SIZE) { printf("Error reading bin file, only got %d bytes", checksum); fclose(BINFile); return 1; }
						fclose(BINFile);
						
#ifdef _VHDDEBUG
						puts("Sector pulled from bin file: ");
						for(count = 0; count < SECTOR_SIZE; count++) printf("%0x ", sector[count]);
						putchar('\n');
#endif
						
						if(!(VHDFile = fopen(VHDPath, "r+b"))) { printf("Could not open file\"%s\"!\n", VHDPath); return 1; }
						if(fseek(VHDFile, sectorOffset, SEEK_SET)) { printf("Error seeking to BAT entry"); fclose(VHDFile); return 1; }
						if(checksum = fwrite(sector, 1, SECTOR_SIZE, VHDFile) < SECTOR_SIZE) { printf("Error writing sector to VHD, only sent %d bytes", checksum); fclose(VHDFile); return 1; }
						
						//sector should have been written, check if bit flag is set, if not do so
						if(!sectorWritten)
						{
							if(fseek(VHDFile, blockOffset + (sectorInBlock / SECTOR_SIZE), SEEK_SET)) { printf("Error seeking to BAT entry"); fclose(VHDFile); return 1; }
							bitmapByte ^= 0x00000001 << (7 - (sectorInBlock % 8));
							
#ifdef _VHDDEBUG
							printf("bitmapByte changed to 0x%x", bitmapByte);
#endif
							if(checksum = fwrite(&bitmapByte, 1, 1, VHDFile) < 1) { printf("Error writing to bitmap, only got %d bytes", checksum); fclose(VHDFile); return 1; }
							//Write to sector in VHD
						}
						fclose(VHDFile);
						break;
					default:
						puts("Should never have gotten here. Invalid operation.");
						return 1;
				}
			}
			break;
		case DISK_TYPE_DIFFERENCING:
			puts("Differencing disks not supported...yet");
			return 1;
		default:
			puts("Unknown disk type");
			return 1;
	}	
	return 0;
}
