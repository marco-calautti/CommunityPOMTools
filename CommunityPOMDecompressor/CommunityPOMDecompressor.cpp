#include <cstdio>
#include <cstdint>
#include <cstring>
#include "lzss.h"

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		printf("Usage:\n");
		printf("%s [-d | -c] input_file output_file\n",argv[0]);
		printf("-d: decompress input file to output file\n");
		printf("-c: compress input file to output file");
		return 0;
	}


	FILE* inf = fopen(argv[2], "rb");

	fseek(inf, 0, SEEK_END);
	int size = ftell(inf);
	fseek(inf, 0, SEEK_SET);

	uint8_t* src = new uint8_t[size];
	fread((void*)src, 1, size, inf);
	fclose(inf);


	uint8_t* dest = NULL;
	int dest_size = 0;

	if (strcmp(argv[1], "-d") == 0)
		dest_size = LzssDecode(dest, src, size);
	else if (strcmp(argv[1], "-c") == 0)
		dest_size = LzssEncode(dest, src, size);
	else
	{
		printf("Invalid option %s\n", argv[1]);
		return -1;
	}

	FILE* of = fopen(argv[3], "wb+");
	fwrite(dest, 1, dest_size, of);
	fclose(of);

	delete[] src;
	delete[] dest;
    return 0;
}

