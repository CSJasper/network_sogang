#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <vector>

#define str(s)	#s
#define print(x) printf("%s\n", str(x))
#define NORETURN assert(false)

enum {
	DIR_INDEX = 0,
	INPUT_INDEX = 1,
	OUTPUT_INDEX = 2,
	RESULT_DIR_INDEX = 3,
	GENERATOR_INDEX = 4,
	DATAWORD_SIZE_INDEX = 5
};

size_t generator_bit;


int main(int argc, char* argv[]) {
	FILE* rstream, * wstream, * resstream;
	size_t dataword_size = 0;
	if (argc != 6) {
		print(usage: ./crc_decoder input_file output_file result_file generator dataword_size);
		exit(-1);
	}
	
	rstream = fopen((const char*)argv[INPUT_INDEX], "rb");
	
	if (rstream == NULL) {
		print(input file open error.);
		exit(-1);
	}

	wstream = fopen((const char*)argv[OUTPUT_INDEX], "wb");

	if (wstream == NULL) {
		print(output file open error.);
		exit(-1);
	}

	resstream = fopen((const char*)argv[RESULT_DIR_INDEX], "wb");

	if (resstream == NULL) {
		print(result file open error.);
		exit(-1);
	}

	if (strcmp((const char*)argv[DATAWORD_SIZE_INDEX], "4") == 0) {
		dataword_size = 4;
	}
	else if (strcmp((const char*)argv[DATAWORD_SIZE_INDEX], "8") == 0) {
		dataword_size = 8;
	}
	else {
		print(dataword size must be 4 or 8.);
		exit(-1);
	}

	size_t pad_size = 0;
	size_t read_num = fread(&pad_size, sizeof(uint8_t), 1, rstream);
	int seek_val = fseek(rstream, 0, SEEK_END);
	size_t file_size = (size_t)ftell(rstream) - 1;
	seek_val = fseek(rstream, 1, SEEK_SET);


	return 0;
}