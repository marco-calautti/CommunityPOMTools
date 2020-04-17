#include "lzss.h"

#include <cstring>
#include <cstdio>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct {
	char magic[3];
	uint8_t is_big_endian;
	uint32_t dest_size;
} rux_header;


bool get_match(uint8_t* src, int src_seek, int src_size, int* match_jump, int* match_len)
{
	int match_seek = ((src_seek - N) < 0) ? 0 : src_seek - N;

	*match_jump = src_seek - match_seek;
	*match_len = 0;
	while (match_seek < src_seek) {
		//searching match
		int search_cursor = match_seek;
		int cur_src_seek = src_seek;
		int cur_length = 0;

		while (	src[search_cursor] == src[cur_src_seek] &&
				cur_src_seek < src_size &&
				cur_length < F) {

			cur_length++;
			cur_src_seek++;
			search_cursor++;
		}
		if (cur_length > *match_len) {
			*match_len = cur_length;
			*match_jump = src_seek - match_seek;
		}
		match_seek++;
	}

	return *match_len >= (THRESHOLD+1);
}

int LzssEncode(uint8_t* &dest, uint8_t* src, int src_size)
{
	dest = new uint8_t[src_size * 2 + 8];

	strcpy((char*)dest, "RUX");
	dest[3] = 0;
	*(uint32_t*)(dest + 4) = src_size;

	int dest_seek = 8;
	int src_seek = 0, flag_pos, i;
	int match_jump, match_len;
	uint16_t flags = 0;

	while (src_seek < src_size)
	{
		flag_pos = dest_seek;
		dest_seek += 2;
		for (i = 0; i < 16; i++)
		{
			if (get_match(src, src_seek, src_size, &match_jump, &match_len))
			{
				if (match_len > 0x10)
				{
					*((uint16_t*)(dest + dest_seek)) = match_jump - 1;
					dest_seek += 2;
					dest[dest_seek++] = match_len - 0x10 - 1;
				}
				else
				{
					*((uint16_t*)(dest + dest_seek)) = ((match_len - 1) << 0xC) | (match_jump - 1);
					dest_seek += 2;
				}

				flags = (flags << 1) | 1;
				src_seek += match_len;
			}
			else
			{
				dest[dest_seek++] = src[src_seek++];
				flags = flags << 1;
			}

			if (src_seek >= src_size)
			{
				flags = flags << (16 - i - 1);
				break;
			}
		}
		
		*((uint16_t*)(dest + flag_pos)) = flags;
		flags = 0;
	}

	return dest_seek;
}

int LzssDecode(uint8_t* &dest, uint8_t* src, int src_size)
{
	int fpos;
	int dest_seek, src_seek;
	uint8_t  c;
	uint32_t flags;

	rux_header* header = (rux_header*)src;
	if (strncmp(header->magic, "RUX", 3))
		return -1;

	int dest_size = header->dest_size;
	if (header->is_big_endian)
		dest_size = ((dest_size & 0xFF) << 0x18) |
					((dest_size & 0xFF00) << 0x8) |
					((dest_size & 0xFF0000) >> 0x8) |
					(dest_size >> 0x18);

	
	dest = new uint8_t[dest_size];

	for (fpos = 0, flags = 0, dest_seek = 0, src_seek = 8; src_seek<src_size && dest_seek<dest_size; flags <<= 1, fpos++)
	{
		// read flag
		if ((fpos %= 16) == 0) {
			flags = *((uint16_t*)(src + src_seek));
			src_seek += 2;
		}

		if (!(flags & 0x8000))
		{
			c = src[src_seek++];
			dest[dest_seek++] = c;
		}
		else
		{
			uint16_t data = *((uint16_t*)(src + src_seek));
			src_seek += 2;

			int j, i;
			if (data & 0xF000) {
				j = data >> 0xC;
				i = (data & 0x0FFF) + 1;
			}
			else
			{
				i = data + 1;
				j = src[src_seek++] + 0x10;
			}
			j += THRESHOLD;
			i = dest_seek - i;
			// decompress string
			for (int k = 0; k < j; k++)
			{
				c = dest[i + k];
				dest[dest_seek++] = c;
			}
		}
	}
	return dest_seek;
}