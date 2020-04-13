#pragma once

#include <cstdint>

#define N        4096   /* max jump */
#define F          272   /* upper limit for match_length */
#define THRESHOLD   1   /* encode string into position and length
						   if match_length is greater or equal than this */

int LzssEncode(uint8_t* &dest, uint8_t* src, int src_size);
int LzssDecode(uint8_t* &dest, uint8_t* src, int src_size);