#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <vector>

#define str(s)	#s
#define print(x) printf("%s\n", str(x))
#define NORETURN assert(false)

/* simple utility function */

inline size_t get_byte_size(const size_t bit_size);
inline uint8_t to_byte(const char ch);


/* bitwise utility functions */

inline void w_bitwise_or_align_left(uint8_t* target, uint8_t* operand, const size_t target_bytes, const size_t operand_bytes);
inline void w_bitwise_and_align_left(uint8_t* target, uint8_t* operand, const size_t target_bytes, const size_t operand_bytes);
inline void w_bitwise_xor_align_left(uint8_t* target, uint8_t* operand, const size_t target_bytes, const size_t operand_bytes);

void shift_left_once(uint8_t* bitmap, const size_t bytes);
void shift_right_once(uint8_t* bitmap, const size_t bytes);
inline void shift_left(uint8_t* bitmap, const size_t bytes, const size_t times);
inline void shift_right(uint8_t* bitmap, const size_t bytes, const size_t times);
inline bool is_msb_one(uint8_t* bitmap);
inline void set_msb(uint8_t* bitmap, uint8_t one_or_zero);

/* memory management function */
inline uint8_t* malloc_bitmap(const size_t bit_size);
inline void free_bitmap(uint8_t* ptr);

/* CRC check */
void divide(dataword_t* dataword, generator_t* gen, dataword_t* r);

std::vector<codeword_t> codewords_list(uint8_t* raw_data, size_t total_bytes);

/*
*raw data를 저장한다
raw data에서 padding 된거 만큼 시작점을 shift한다.(logical shift) 절대 메모리를 shift 하는게 아님

raw data를 가지고 codeword를 만든다
만든 codeword를 가지고 crc check를 한다 -> generator로 나눴을때 나머지가 0인지 확인
crc check를 할때 오류 개수를 count한다

codeword를 가지고 앞의 dataword부분만 bitwise하게 복사한다.

만약 dataword size가 4 비트라면 두개씩 모아서 하나의 바이트에 bitwise copy 한다. (하나를 copy 그리고 shift 후 copy: for example)
만약 dataword size가 8 비트라면 codeword 하나당 하나의 바이트에 bitwise copy 한다.

*/


typedef struct _dataword {
	size_t unused_bit;
	size_t total_bit;
	size_t byte_size;
	uint8_t* data;
}dataword_t;

typedef struct _codeword {
	size_t left_pad;
	size_t byte_size;
	uint8_t* data;
}codeword_t;

typedef struct _generator {
	size_t unused_bit;
	size_t total_byte;
	uint8_t* data;
}generator_t;

enum {
	DIR_INDEX = 0,
	INPUT_INDEX = 1,
	OUTPUT_INDEX = 2,
	RESULT_DIR_INDEX = 3,
	GENERATOR_INDEX = 4,
	DATAWORD_SIZE_INDEX = 5
};

size_t generator_bit;
size_t codeword_bit;
size_t codeword_count;
size_t error_count;


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

	uint8_t* raw_data = malloc_bitmap(file_size);

	std::vector<uint8_t> gc;
	gc.reserve(file_size);

	generator_bit = strlen((const char*)argv[GENERATOR_INDEX]);
	codeword_bit = dataword_size + generator_bit - 1;
	size_t codeword_byte = get_byte_size(codeword_bit);

	size_t read_num = fread(raw_data, sizeof(uint8_t), file_size, rstream);

	shift_left(raw_data, file_size, pad_size);


	std::vector<codeword_t> codewords;




	/* memory deallocation */
	free_bitmap(raw_data);
	fclose(rstream);
	fclose(wstream);
	fclose(resstream);

	return 0;
}

inline void w_bitwise_or_align_left(uint8_t* target, uint8_t* operand, const size_t target_bytes, const size_t operand_bytes) {
	assert(target_bytes >= operand_bytes);
	for (size_t i = 0; i < operand_bytes; i++) {
		target[i] |= operand[i];
	}
}

inline void w_bitwise_and_align_left(uint8_t* target, uint8_t* operand, const size_t target_bytes, const size_t operand_bytes) {
	assert(target_bytes >= operand_bytes);
	for (size_t i = 0; i < operand_bytes; i++) {
		target[i] &= operand[i];
	}
}

inline void w_bitwise_xor_align_left(uint8_t* target, uint8_t* operand, const size_t target_bytes, const size_t operand_bytes) {
	assert(target_bytes >= operand_bytes);
	for (size_t i = 0; i < operand_bytes; i++) {
		target[i] ^= operand[i];
	}
}

void shift_left_once(uint8_t* bitmap, const size_t bytes) {
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

void shift_right_once(uint8_t* bitmap, const size_t bytes) {
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

inline void shift_left(uint8_t* bitmap, const size_t bytes, const size_t times) {
	for (size_t i = 0; i < times; i++) {
		shift_left_once(bitmap, bytes);
	}
}

inline void shift_right(uint8_t* bitmap, const size_t bytes, const size_t times) {
	for (size_t i = 0; i < times; i++) {
		shift_right_once(bitmap, bytes);
	}
}

inline uint8_t* malloc_bitmap(const size_t bit_size) {
	size_t byte_size = get_byte_size(bit_size);
	return (uint8_t*)malloc(sizeof(uint8_t) * byte_size);
}

inline void free_bitmap(uint8_t* ptr) {
	free(ptr);
}

inline size_t get_byte_size(const size_t bit_size) {
	size_t bytes = 0;
	if (bit_size % 8 == 0)
		bytes = bit_size / 8;
	else
		bytes = (bit_size / 8) + 1;
	return bytes;
}

/* r->data should be memory allocated and should be initialized as zero bitmap */
void divide(dataword_t* dataword, generator_t* gen, dataword_t* r) {
	assert(r->data != NULL);
	assert(dataword->byte_size == r->byte_size);
	memcpy(r->data, dataword->data, dataword->byte_size);

	for (size_t i = 0; i < dataword->total_bit - generator_bit + 1; i++) {
		if (is_msb_one(r->data)) {
			w_bitwise_xor_align_left(r->data, gen->data, r->byte_size, gen->total_byte);
		}
		shift_left_once(r->data, r->byte_size);
		r->unused_bit += 1;
		r->total_bit -= 1;
	}
}

inline bool is_msb_one(uint8_t* bitmap) {
	uint8_t mask = 0x80;
	switch (*bitmap & mask) {
	case 0x80:
		return true;
	case 0x00:
		return false;
	default:
		NORETURN;
	}
}

inline uint8_t to_byte(const char ch) {
	uint8_t bit = 0x00;

	switch (ch) {
	case '1':
		bit = 0x01;
		break;
	case '0':
		bit = 0x00;
	default:
		NORETURN;
	}
	return bit;
}

generator_t construct_generator(const char* gen_str) {
	generator_t gen;
	size_t bits = generator_bit;
	size_t bytes = get_byte_size(bits);
	uint8_t* data = malloc_bitmap(bits);
	for (size_t i = strlen(gen_str); i > 1; i--) {
		size_t it = i - 1;
		set_msb(data, to_byte(gen_str[it]));
		shift_right_once(data, bytes);
	}
	set_msb(data, to_byte(gen_str[0]));
	gen.data = data;
	gen.total_byte = bytes;
	gen.unused_bit = bytes * 8 - bits;
	return gen;
}

inline void set_msb(uint8_t* bitmap, uint8_t one_or_zero) {
	if (one_or_zero == 1)
		bitmap[0] ^= 0x80;
	else
		bitmap[0] &= 0x7f;
}

