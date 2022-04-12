#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define str(s)	#s
#define print(x) printf("%s\n", str(x))

enum {
	RSTREAM_INDEX = 1,
	WSTREAM_INDEX = 2,
	GENERATOR_INDEX = 3,
	DWORD_INDEX = 4
};

FILE* rstream, * wstream;

typedef struct _dataword {
	uint8_t* data;
	size_t unused_bit;
	size_t total_bit;
	size_t byte_size;
}dataword_t;

typedef struct _codeword {
	uint8_t* data;
	size_t left_pad;
	size_t byte_size;
}codeword_t;

typedef struct _generator {
	uint8_t* data;
	size_t unused_bit;
	size_t total_byte;
}generator_t;

/*
*	structure :
*
*/

/* utility function */
inline uint8_t to_byte(const char ch);


inline size_t get_byte_size(const size_t bit_size);
uint8_t* malloc_bitmap(const size_t bit_size);
inline void free_bitmap(uint8_t*);
dataword_t construct_dataword(uint8_t* raw_data, size_t dataword_bit);
codeword_t construct_codeword(dataword_t* dataword, dataword_t* remainder);
void divide(dataword_t dataword, generator_t* gen, dataword_t* r);
void shift_left_once(uint8_t* bitmap, size_t bytes);
void shift_right_once(uint8_t* bitmap, size_t bytes);
inline void shift_left(uint8_t* bitmap, size_t bytes, const size_t times);
inline void shift_right(uint8_t* bitmap, size_t bytes, const size_t times);
generator_t construct_generator(const char* gen_str);

/* bitmap operation */
inline void w_bitwise_or_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes);
inline void w_bitwise_and_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes);
inline void w_bitwise_xor_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes);
inline void set_msb(uint8_t* bitmap, uint8_t one_or_zero);


size_t generator_bit;
/* for debug implement dump functions */


int main(int argc, char* argv[]) {
	/* error handling */
	if (argc != 4) {
		print(usage: . / crc_decoder input_file output_file generator dataword_size);
		exit(-1);
	}

	size_t dataword_size = 0;


	if (strcmp(argv[DWORD_INDEX], "4") == 0) {
		dataword_size = 4;
	}
	else if (strcmp(argv[DWORD_INDEX], "8") == 0) {
		dataword_size = 8;
	}
	else {
		print(dataword size must be 4 or 8.);
		exit(-1);
	}

	rstream = (FILE*)fopen(argv[RSTREAM_INDEX], "rb");
	if (rstream == NULL) {
		print(input file open error.);
		exit(-1);
	}

	wstream = (FILE*)fopen(argv[WSTREAM_INDEX], "wb");

	if (wstream == NULL) {
		print(output file open error.);
		fclose(rstream);
		rstream = NULL;
		exit(-1);
	}



	/* set codeword by bitwise division */

	/* final steps to free memory and stream */


	fclose(rstream);
	rstream = NULL;
	fclose(wstream);
	wstream = NULL;

	return 0;
}

inline size_t get_byte_size(const size_t bit_size) {
	size_t bytes = 0;
	if (bit_size % 8 == 0)
		bytes = bit_size / 8;
	else
		bytes = (bit_size / 8) + 1;
	return bytes;
}

uint8_t* malloc_bitmap(const size_t bit_size) {
	size_t byte = get_byte_size(bit_size);
	uint8_t* bitmap = (uint8_t*)malloc(sizeof(uint8_t) * byte);
	for (size_t i = 0; i < byte; i++) {
		bitmap[i] ^= bitmap[i];
	}
	return bitmap;
}

inline void free_bitmap(uint8_t* p) {
	free(p);
}

/* */
dataword_t construct_dataword(uint8_t* raw_data, size_t dataword_bit) {
	dataword_t dword;
	size_t bytes = get_byte_size(dataword_bit + generator_bit - 1);
	uint8_t* bitmap = malloc_bitmap(dataword_bit + generator_bit - 1);
	/* copy data to bitmap aligned left */
	w_bitwise_or_align_left(bitmap, raw_data, bytes, dataword_bit / 8);
	dword.data = bitmap;
	dword.byte_size = bytes;
	dword.total_bit = dataword_bit + generator_bit - 1;
	dword.unused_bit = dword.byte_size * 8 - dword.total_bit;
	return dword;
}

codeword_t construct_codeword(dataword_t* dataword, dataword_t* remainder) {
	size_t dataword_bit = dataword->total_bit - (generator_bit - 1);
	shift_right(remainder->data, remainder->byte_size, dataword_bit);
	w_bitwise_or_align_left(dataword->data, remainder->data, dataword->byte_size, remainder->byte_size);
	uint8_t* data = malloc_bitmap(dataword->total_bit);
	codeword_t codeword;
	memcpy(data, dataword->data, dataword->byte_size);
	shift_right(data, dataword->byte_size, dataword->unused_bit);
	codeword.data = data;
	codeword.byte_size = dataword->byte_size;
	codeword.left_pad = dataword->unused_bit;
	return codeword;
}

void divide(dataword_t dataword, generator_t* gen, dataword_t* r) {

}

void shift_left_once(uint8_t* bitmap, size_t bytes) {
	uint8_t carry = 0x00;
	for (size_t i = bytes; i > 0; i--) {
		/* save msb for current bytes */
		uint8_t msb = 0x80;
		size_t it = i - 1;
		msb &= bitmap[it];
		/* shift by bytes */
		bitmap[it] = bitmap[it] << 1;
		/* apply carry */
		bitmap[it] |= carry;
		/* update carry */
		carry = msb >> 7;
	}
}

void shift_right_once(uint8_t* bitmap, size_t bytes) {
	uint8_t borrow = 0x00;
	for (size_t i = 0; i < bytes; i++) {
		/* save lsb for current bytes */
		uint8_t lsb = 0x01;
		lsb &= bitmap[i];
		/* shift by bytes */
		bitmap[i] = bitmap[i] >> 1;
		/* apply borrow */
		bitmap[i] |= borrow;
		borrow = lsb << 7;
	}
}

inline void shift_left(uint8_t* bitmap, size_t bytes, const size_t times) {
	for (size_t i = 0; i < times; i++) {
		shift_left_once(bitmap, bytes);
	}
}

inline void shift_right(uint8_t* bitmap, size_t bytes, const size_t times) {
	for (size_t i = 0; i < times; i++) {
		shift_right_once(bitmap, bytes);
	}
}

generator_t construct_generator(const char* gen_str) {
	generator_t gen;
	size_t bits = strlen(gen_str);
	size_t bytes = get_byte_size(bits);
	uint8_t* data = malloc_bitmap(bits);
	for (size_t i = strlen(gen_str); i > 0; i--) {
		size_t it = i - 1;
		set_msb(data, to_byte(gen_str[it]));
		shift_right_once(data, bytes);
	}
	gen.data = data;
	gen.total_byte = bytes;
	gen.unused_bit = bytes * 8 - bits;
	return gen;
}

inline void w_bitwise_or_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes) {
	assert(target_bytes >= operand_bytes);
	for (size_t i = 0; i < operand_bytes; i++) {
		target[i] |= operand[i];
	}
}

inline void w_bitwise_and_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes) {
	assert(target_bytes >= operand_bytes);
	for (size_t i = 0; i < operand_bytes; i++) {
		target[i] &= operand[i];
	}
}

inline void w_bitwise_xor_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes) {
	assert(target_bytes >= operand_bytes);
	for (size_t i = 0; i < operand_bytes; i++) {
		target[i] ^= operand[i];
	}
}

inline void set_msb(uint8_t* bitmap, uint8_t one_or_zero) {
	assert(one_or_zero == 1 || one_or_zero == 0);
	if (one_or_zero == 1)
		bitmap[0] ^= 0x80;
	else
		bitmap[0] ^= 0x00;
}

inline uint8_t to_byte(const char ch) {
	uint8_t bit = 0x00;
	if (ch == '1') {
		bit = 0x01;
	}
	else if (ch == '0') {
		bit = 0x00;
	}
	else {
		assert(false);
	}
	return bit;
}