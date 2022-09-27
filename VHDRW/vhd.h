#include "bswap.h"

#define FOOTER_COOKIE_OFFSET 0
#define FEATURES_OFFSET 8
#define FORMAT_VERSION_MAJOR_OFFSET 12
#define FORMAT_VERSION_MINOR_OFFSET 14
#define FOOTER_DATA_OFFSET_OFFSET 16
#define TIME_STAMP_OFFSET 24
#define CREATOR_APP_OFFSET 28
#define CREATOR_VERSION_MAJOR_OFFSET 32
#define CREATOR_VERSION_MINOR_OFFSET 34
#define CREATOR_OS_OFFSET 36
#define ORIGINAL_SIZE_OFFSET 40
#define CURRENT_SIZE_OFFSET 48
#define GEOMETRY_CYLINDER_OFFSET 56
#define GEOMETRY_HEAD_OFFSET 58
#define GEOMETRY_SECTOR_OFFSET 59
#define DISK_TYPE_OFFSET 60
#define FOOTER_CHECKSUM_OFFSET 64
#define FOOTER_CHECKSUM_LASTBYTE 67

#define TABLE_OFFSET 16
#define MAX_TABLE_ENTRIES_OFFSET 28
#define BLOCK_SIZE_OFFSET 32
#define HEADER_CHECKSUM_OFFSET 36
#define HEADER_CHECKSUM_LASTBYTE 39

#define COOKIE_SIZE 8
#define FEATURES_SIZE 4
#define FORMAT_VERSION_SIZE 4
#define OFFSET_SIZE 8
#define TIME_STAMP_SIZE 4
#define CREATOR_APP_SIZE 4
#define CREATOR_VERSION_SIZE 4
#define CREATOR_OS_SIZE 4
#define LENGTH_SIZE 8

#define FEATURE_RESERVED 2
#define FEATURE_TEMPORARY 1

#define CREATOR_OS_WINDOWS "Wi2k"
#define CREATOR_OS_MAC "Mac "

#define FOOTER_SIZE 512
#define HEADER_SIZE 1024
#define LEGACY_FOOTER_SIZE 511
#define SECTOR_SIZE 512
#define BAT_ENTRY_SIZE 4

#define DISK_TYPE_FIXED 2
#define DISK_TYPE_DYNAMIC 3
#define DISK_TYPE_DIFFERENCING 4

void printVHDFooter(uint8_t *footer)
{
	int count;
	for(count = 0; count < FOOTER_SIZE; count++) printf("%d:%d ", count, *(footer + count));
}

char *metricFormat(uint32_t num)
{
	uint32_t numdiv;
	uint8_t count;
	static char buf[8];
	char metric[5][3] = {"B", "KB", "MB", "GB", "TB"};
	count = 0;
	while(numdiv > 0x400)
	{
		numdiv /= 0x400;
		count ++;
	}
	sprintf(buf, "%d %s", numdiv, metric[count]);
	return buf;
}

uint32_t validateVHDFooterChecksum(void *footer)
{
	uint32_t checksum = 0;
	int count;
	uint32_t footerChecksum;
	
	checksum = 0;
	for(count = 0; count < FOOTER_SIZE; count++)
	{
		if(count < FOOTER_CHECKSUM_OFFSET || count > FOOTER_CHECKSUM_LASTBYTE) checksum += *((uint8_t*)footer + count);
	}
	
	footerChecksum = bswap_32(*(uint32_t*)((uint8_t*)footer + FOOTER_CHECKSUM_OFFSET));
	return footerChecksum ^ checksum;
}

int getVHDType(void *footer)
{
	uint32_t *disktype;
	disktype = (uint32_t *)((uint8_t *)footer + DISK_TYPE_OFFSET);
	return bswap_32(*disktype);
}

uint64_t getVHDHeaderOffset(void *footer)
{
	uint64_t *header;
	header = (uint64_t *)((uint8_t*)footer + FOOTER_DATA_OFFSET_OFFSET);
	return bswap_64(*header);
}

char *getVHDHeaderCookie(void *header)
{
	char cookie[9];
	strncpy(cookie, header, 8);
	cookie[8] = 0;
	return cookie;
}

uint32_t validateVHDHeaderChecksum(void *header)
{
	uint32_t checksum;
	int count;
	uint32_t headerChecksum;
	
	checksum = 0;
	for(count = 0; count < HEADER_SIZE; count++)
	{
		if(count < HEADER_CHECKSUM_OFFSET || count > HEADER_CHECKSUM_LASTBYTE) checksum += *((uint8_t*)header + count);
	}
	headerChecksum = bswap_32(*(uint32_t*)((uint8_t*)header + HEADER_CHECKSUM_OFFSET));
	return headerChecksum ^ ~checksum;
}

uint32_t getVHDMaxTableEntries(void *header)
{
	uint32_t max;
	max = *(uint32_t *)((uint8_t*)header + MAX_TABLE_ENTRIES_OFFSET);
	return bswap_32(max);
}

uint64_t getVHDTableOffset(void * header)
{
	uint64_t table;
	table = *(uint64_t *)((uint8_t*)header + TABLE_OFFSET);
	return bswap_64(table);	
}

uint32_t getVHDBlockSize(void *header)
{
	uint32_t blockSize;
	blockSize = *(uint32_t *)((uint8_t*)header + BLOCK_SIZE_OFFSET);
	return bswap_32(blockSize);
}
