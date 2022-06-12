#include "class.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constant_pool_t parse_constant_pool(uint8_t** data) {
	uint16_t constant_count = BSWAP16(*(uint16_t*) *data);
	*data += 2;
	constant_pool_t constant_pool;
	constant_pool.constants = malloc((constant_count - 1) * sizeof(constant_t));
	constant_pool.constant_count = constant_count - 1;
	for (uint16_t i = 0; i < constant_count - 1; ++i) {
		uint8_t tag = **data;
		*data += 1;
		constant_t* constant = constant_pool.constants + i;
		constant->tag = tag;
		switch (tag) {
			case 1: // Utf8
			{
				uint16_t length = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->string = malloc(length + 1);
				memcpy(constant->string, *data, length);
				constant->string[length] = 0;
				*data += length;
				break;
			}
			case 3: // Integer
			{
				uint32_t bytes = *(uint32_t*) *data;
				*data += 4;
				constant->int_value = (int32_t) BSWAP32(bytes);
				break;
			}
			case 4: // Float
			{
				uint32_t bytes = *(uint32_t*) *data;
				*data += 4;
				bytes = BSWAP32(bytes);
				constant->float_value = *(float*) &bytes;
				break;
			}
			case 5: // Long
			{
				uint32_t high_bytes = BSWAP32(*(uint32_t*) *data);
				*data += 4;
				uint32_t low_bytes = BSWAP32(*(uint32_t*) *data);
				*data += 4;
				int64_t value = ((int64_t) high_bytes << 32) + low_bytes;
				constant->long_value = value;
				++i;
				break;
			}
			case 6: // Double
			{
				uint32_t high_bytes = BSWAP32(*(uint32_t*) *data);
				*data += 4;
				uint32_t low_bytes = BSWAP32(*(uint32_t*) *data);
				*data += 4;
				int64_t value = ((int64_t) high_bytes << 32) + low_bytes;
				constant->double_value = *(double*) &value;
				++i;
				break;
			}
			case 7: // Class
			{
				uint16_t name_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->name_index = name_index;
				break;
			}
			case 8: // String
			{
				uint16_t string_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->string_index = string_index;
				break;
			}
			case 9: // FieldRef
			case 10: // MethodRef
			case 11: // InterfaceMethodRef
			{
				uint16_t class_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				uint16_t name_and_type_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->class_index = class_index;
				constant->name_and_type_index = name_and_type_index;
				break;
			}
			case 12: // NameAndType
			{
				uint16_t name_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				uint16_t descriptor_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->name_index = name_index;
				constant->descriptor_index = descriptor_index;
				break;
			}
			case 15: // MethodHandle
			{
				uint8_t reference_kind = **data;
				++*data;
				uint16_t reference_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->reference_kind = reference_kind;
				constant->reference_index = reference_index;
				break;
			}
			case 16: // MethodType
			{
				uint16_t descriptor_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->descriptor_index = descriptor_index;
				break;
			}
			case 17: // Dynamic
			case 18: // InvokeDynamic
			{
				uint16_t bootstrap_method_attr_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				uint16_t name_and_type_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->bootstrap_method_attr_index = bootstrap_method_attr_index;
				constant->name_and_type_index = name_and_type_index;
				break;
			}
			case 19: // Module
			{
				uint16_t name_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->name_index = name_index;
				break;
			}
			case 20: // Package
			{
				uint16_t name_index = BSWAP16(*(uint16_t*) *data);
				*data += 2;
				constant->name_index = name_index;
				break;
			}
			default:
				break;
		}
	}
	return constant_pool;
}

attribute_t* parse_attributes(uint8_t** data, constant_pool_t constant_pool) {
	uint16_t attribute_count = BSWAP16(*(uint16_t*) *data);
	*data += 2;
	if (attribute_count == 0) return NULL;
	attribute_t* attributes = malloc(attribute_count * sizeof(attribute_t));

	for (uint16_t i = 0; i < attribute_count; ++i) {
		uint16_t attribute_name_index = BSWAP16(*(uint16_t*) *data);
		*data += 2;
		uint32_t attribute_length = BSWAP32(*(uint32_t*) *data);
		*data += 4;
		attributes[i].name = constant_pool.constants[attribute_name_index - 1].string;
		attributes[i].length = attribute_length;
		if (attributes->length == 0) continue;
		attributes[i].data = malloc(attribute_length);
		memcpy(attributes[i].data, *data, attribute_length);
		*data += attribute_length;
	}

	return attributes;
}

field_t* parse_fields(uint8_t** data, constant_pool_t constant_pool) {
	uint16_t field_count = BSWAP16(*(uint16_t*) *data);
	*data += 2;
	if (field_count == 0) return NULL;
	field_t* fields = malloc(field_count * sizeof(field_t));
	for (uint16_t i = 0; i < field_count; ++i) {
		uint16_t access_flags = BSWAP16(*(uint16_t*) *data);
		*data += 2;
		uint16_t name_index = BSWAP16(*(uint16_t*) *data);
		*data += 2;
		uint16_t descriptor_index = BSWAP16(*(uint16_t*) *data);
		*data += 2;
		uint16_t attribute_count = BSWAP16(*(uint16_t*) *data);
		attribute_t* attributes = parse_attributes(data, constant_pool);

		uint8_t* name = constant_pool.constants[name_index - 1].string;
		uint8_t* descriptor = constant_pool.constants[descriptor_index - 1].string;

		fields[i].access_flags = access_flags;
		fields[i].name = name;
		fields[i].descriptor = descriptor;
		fields[i].attribute_count = attribute_count;
		fields[i].attributes = attributes;
	}
	return fields;
}

uint8_t** parse_interfaces(uint8_t** data, constant_pool_t constant_pool) {
	uint16_t interface_count = BSWAP16(*(uint16_t*) *data);
	*data += 2;
	if (interface_count == 0) return NULL;
	uint8_t** interfaces = malloc(interface_count * sizeof(uint8_t*));

	for (uint16_t i = 0; i < interface_count; ++i) {
		uint16_t index = BSWAP16(*(uint16_t*) *data);
		*data += 2;
		interfaces[i] = constant_pool.constants[constant_pool.constants[index - 1].name_index - 1].string;
	}
	return interfaces;
}

class_t* load_class(const char* filepath) {
	FILE* file = fopen(filepath, "r");
	if (!file) return NULL;

	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t* data = malloc(file_size);
	uint8_t* ptr = data;
	fread(data, file_size, 1, file);
	fclose(file);

	uint32_t magic = BSWAP32(*(uint32_t*) data);
	data += 4;
	uint16_t minor_version = BSWAP16(*(uint16_t*) data);
	data += 2;
	uint16_t major_version = BSWAP16(*(uint16_t*) data);
	data += 2;
	constant_pool_t constant_pool = parse_constant_pool(&data);
	uint16_t access_flags = BSWAP16(*(uint16_t*) data);
	data += 2;
	uint16_t this_class = BSWAP16(*(uint16_t*) data);
	data += 2;
	uint16_t super_class = BSWAP16(*(uint16_t*) data);
	data += 2;
	uint16_t interface_count = BSWAP16(*(uint16_t*) data);
	uint8_t** interfaces = parse_interfaces(&data, constant_pool);
	uint16_t field_count = BSWAP16(*(uint16_t*) data);
	field_t* fields = parse_fields(&data, constant_pool);
	uint16_t method_count = BSWAP16(*(uint16_t*) data);
	field_t* methods = parse_fields(&data, constant_pool);
	uint16_t attribute_count = BSWAP16(*(uint16_t*) data);
	attribute_t* attributes = parse_attributes(&data, constant_pool);

	free(ptr);

	class_t* class = malloc(sizeof(class_t));
	class->magic = magic;
	class->minor_version = minor_version;
	class->major_version = major_version;
	class->constant_pool = constant_pool;
	class->access_flags = access_flags;
	class->this_class = constant_pool.constants[constant_pool.constants[this_class - 1].name_index - 1].string;
	class->super_class = constant_pool.constants[constant_pool.constants[super_class - 1].name_index - 1].string;
	class->interface_count = interface_count;
	class->interfaces = interfaces;
	class->field_count = field_count;
	class->fields = fields;
	class->method_count = method_count;
	class->methods = methods;
	class->attribute_count = attribute_count;
	class->attributes = attributes;
	return class;
}

void class_delete(class_t* class) {
	if (class == NULL) return;
	for (uint16_t i = 0; i < class->constant_pool.constant_count; ++i) {
		if (class->constant_pool.constants[i].tag == 1) {
			free(class->constant_pool.constants[i].string);
		}
	}
	free(class->constant_pool.constants);
	for (uint16_t i = 0; i < class->attribute_count; ++i) {
		free(class->attributes[i].data);
	}
	if (class->attributes != NULL) {
		free(class->attributes);
	}
	if (class->fields != NULL) {
		free(class->fields);
	}
	if (class->interfaces != NULL) {
		free(class->interfaces);
	}
	free(class);
}