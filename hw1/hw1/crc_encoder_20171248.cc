#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#define str(s)	#s
#define print(x) printf("%s\n", str(x))
#define NORETURN assert(false)

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
codeword_t construct_codeword(dataword_t dataword[], size_t dataword_len);
void divide(dataword_t* dataword, generator_t* gen, dataword_t* r);
void shift_left_once(uint8_t* bitmap, size_t bytes);
void shift_right_once(uint8_t* bitmap, size_t bytes);
inline void shift_left(uint8_t* bitmap, size_t bytes, const size_t times);
inline void shift_right(uint8_t* bitmap, size_t bytes, const size_t times);
generator_t construct_generator(const char* gen_str);
dataword_t construct_remainder(size_t dataword_bit);
void add_remainder(dataword_t* dword, dataword_t* remainder);

/* bitmap operation */
inline void w_bitwise_or_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes);
inline void w_bitwise_and_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes);
inline void w_bitwise_xor_align_left(uint8_t* target, uint8_t* operand, size_t target_bytes, size_t operand_bytes);
inline void set_msb(uint8_t* bitmap, uint8_t one_or_zero);
inline void set_lsb(uint8_t* bitmap, uint8_t one_or_zero, size_t byte_size);
inline uint8_t* zeros(size_t bit_size);
inline bool cmp_msb(uint8_t* bitmap1, uint8_t* bitmap2);
inline bool is_msb_one(uint8_t* bitmap);


size_t generator_bit;
/* for debug implement dump functions */


int main(int argc, char* argv[]) {
	/* error handling */
	if (argc != 5) {
		print(usage: ./crc_decoder input_file output_file generator dataword_size);
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

	int seek_val = fseek(rstream, 0, SEEK_END);
	size_t file_size = (size_t)ftell(rstream);
	seek_val = fseek(rstream, 0, SEEK_SET);

	std::vector<uint8_t> buffer(file_size);
	std::vector<dataword_t> datawords;
	std::vector<uint8_t*> gc;
	gc.reserve(file_size);
	datawords.reserve(file_size);


	size_t read_num = fread(&buffer[0], sizeof(uint8_t), file_size, rstream);

	generator_t divisor = construct_generator((const char*)argv[GENERATOR_INDEX]);
	generator_bit = strlen((const char*)argv[GENERATOR_INDEX]);

	/* make dataword */
	if (dataword_size == 8) {
		for (size_t i = 0; i < buffer.size(); i++) {
			uint8_t shot = buffer[i];
			dataword_t dword = construct_dataword(&shot, dataword_size);
			dataword_t remainder = construct_remainder(dword.total_bit);
			divide(&dword, &divisor, &remainder);
			gc.push_back(remainder.data);
			add_remainder(&dword, &remainder);
			datawords.push_back(dword);
		}
	}
	else {
		for (size_t i = 0; i < buffer.size(); i++) {
			uint8_t shot = buffer[i] & 0xf0;
			dataword_t dword1 = construct_dataword(&shot, dataword_size);
			dataword_t r1 = construct_remainder(dword1.total_bit);
			divide(&dword1, &divisor, &r1);
			gc.push_back(r1.data);
			add_remainder(&dword1, &r1);
			datawords.push_back(dword1);

			shot = (buffer[i] & 0x0f) << 4;
			dataword_t dword2 = construct_dataword(&shot, dataword_size);
			dataword_t r2 = construct_remainder(dword2.total_bit);
			divide(&dword2, &divisor, &r2);
			gc.push_back(r2.data);
			add_remainder(&dword2, &r2);
			datawords.push_back(dword2);
		}
	}

	codeword_t cword = construct_codeword(&datawords[0], datawords.size());

	/* write padded num in bytes it should be in range of uint8_t otherwise data loss would occur */
	uint8_t padded_num = (uint8_t)cword.left_pad;
	size_t write_num = fwrite(&padded_num, sizeof(uint8_t), 1, wstream);
	write_num = fwrite(cword.data, sizeof(uint8_t), cword.byte_size, wstream);

	/* final steps to free memory and stream */
	free_bitmap(divisor.data);
	for (size_t i = 0; i < datawords.size(); i++) {
		free_bitmap(datawords[i].data);
	}
	for (size_t i = 0; i < gc.size(); i++) {
		free_bitmap(gc[i]);
	}
	
	free_bitmap(cword.data);
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

/* dataword with 4bit size should be called twice with left aligned bit */
dataword_t construct_dataword(uint8_t* raw_data, size_t dataword_bit) {
	dataword_t dword;
	size_t bytes = get_byte_size(dataword_bit + generator_bit - 1);
	uint8_t* bitmap = malloc_bitmap(dataword_bit + generator_bit - 1);
	/* copy data to bitmap aligned left */
	w_bitwise_or_align_left(bitmap, raw_data, bytes, dataword_bit / 8 + 1);
	dword.data = bitmap;
	dword.byte_size = bytes;
	dword.total_bit = dataword_bit + generator_bit - 1;
	dword.unused_bit = dword.byte_size * 8 - dword.total_bit;
	return dword;
}

/* construct formatted codeword the parameters will be meaningless after execution of this function */
codeword_t construct_codeword(dataword_t dataword[], size_t dataword_len) {
	size_t codeword_bits = dataword[0].total_bit * dataword_len;
	size_t codeword_bytes = get_byte_size(codeword_bits);
	codeword_t cword;

	cword.data = malloc_bitmap(codeword_bits);
	cword.byte_size = codeword_bytes;
	cword.left_pad = codeword_bytes * 8 - codeword_bits;


	/* 현재 data의 msb와 cword.data의 lsb를 같게 한다. */
	for (size_t i = 0; i < dataword_len - 1; i++) {
		for (size_t j = 0; j < dataword[i].total_bit; j++) {
			if (is_msb_one(dataword[i].data)) {
				set_lsb(cword.data, 1, cword.byte_size);
			}
			shift_left_once(cword.data, cword.byte_size);
			shift_left_once(dataword[i].data, dataword[i].byte_size);
		}
	}
	for (size_t j = 0; j < dataword[dataword_len - 1].total_bit - 1; j++) {
		if (is_msb_one(dataword[dataword_len - 1].data)) {
			set_lsb(cword.data, 1, cword.byte_size);
		}
		shift_left_once(cword.data, cword.byte_size);
		shift_left_once(dataword[dataword_len - 1].data, dataword[dataword_len - 1].byte_size);
	}
	if (is_msb_one(dataword[dataword_len - 1].data)) {
		set_lsb(cword.data, 1, cword.byte_size);
	}
	return cword;
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

dataword_t construct_remainder(size_t dataword_bit) {
	dataword_t r;
	size_t bytes = get_byte_size(dataword_bit);
	uint8_t* data = malloc_bitmap(dataword_bit);
	r.data = data;
	r.byte_size = bytes;
	r.total_bit = dataword_bit;
	r.unused_bit = bytes * 8 - dataword_bit;
	return r;
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
	if (one_or_zero == 1)
		bitmap[0] ^= 0x80;
	else
		bitmap[0] &= 0x7f;
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

inline uint8_t* zeros(size_t bit_size) {
	return malloc_bitmap(bit_size);
}

inline bool cmp_msb(uint8_t* bitmap1, uint8_t* bitmap2) {
	uint8_t cmp = *bitmap1 ^ *bitmap2;
	uint8_t mask = 0x80;
	switch (cmp & mask) {
	case 0x80:
		return false;
	case 0x00:
		return true;
	default:
		NORETURN;
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

inline void set_lsb(uint8_t* bitmap, uint8_t one_or_zero, size_t byte_size) {
	if (one_or_zero == 1) {
		bitmap[byte_size - 1] |= 0x01;
	}
	else {
		bitmap[byte_size - 1] &= 0x7f;
	}
}

void add_remainder(dataword_t* dword, dataword_t* remainder) {
	size_t shift_times = dword->total_bit - remainder->total_bit;
	shift_right(remainder->data, remainder->byte_size, shift_times);
	w_bitwise_or_align_left(dword->data, remainder->data, dword->byte_size, remainder->byte_size);
}