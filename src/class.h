#pragma once
#include <stdint.h>
#include <stdlib.h>

#define BSWAP16(x) __builtin_bswap16(x)
#define BSWAP32(x) __builtin_bswap32(x)

typedef struct {
	uint8_t tag;
	union {
		uint16_t name_index;
		uint16_t class_index;
		uint16_t string_index;
		int32_t int_value;
		int64_t long_value;
		float float_value;
		double double_value;
		uint8_t* string;
		uint8_t reference_kind;
		uint16_t bootstrap_method_attr_index;
	};
	union {
		uint16_t name_and_type_index;
		uint16_t descriptor_index;
		uint16_t reference_index;
	};
} constant_t;

typedef struct {
	uint8_t* name;
	uint32_t length;
	uint8_t* data;
} attribute_t;

typedef struct {
	uint16_t access_flags;
	uint8_t* name;
	uint8_t* descriptor;
	uint16_t attribute_count;
	attribute_t* attributes;
} field_t;

typedef struct {
	constant_t* constants;
	uint16_t constant_count;
} constant_pool_t;

typedef struct {
	uint32_t magic;
	uint16_t major_version;
	uint16_t minor_version;
	constant_pool_t constant_pool;
	uint16_t access_flags;
	uint8_t* this_class;
	uint8_t* super_class;
	uint16_t interface_count;
	uint8_t** interfaces;
	uint16_t field_count;
	field_t* fields;
	uint16_t method_count;
	field_t* methods;
	uint16_t attribute_count;
	attribute_t* attributes;
} class_t;

void class_delete(class_t* class);

class_t* load_class(const char* filepath);