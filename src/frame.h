#pragma once
#include <stdint.h>
#include "class.h"
#include "vm.h"

typedef struct class_instance_t {
	class_t* class;
	field_t* fields;
	uint16_t field_count;
} class_instance_t;

typedef struct {
	uint16_t start;
	uint16_t end;
	uint16_t handler;
	uint16_t type;
} jvm_exception_handler_t;

typedef struct jvm_frame_t {
	class_instance_t* instance;
	jvm_type_t* locals;
	uint16_t locals_size;
	jvm_type_t* stack;
	uint16_t ip;
	uint16_t sp;
	uint8_t* code;
	jvm_exception_handler_t* exception_handlers;
	uint16_t exception_handler_count;
} jvm_frame_t;

jvm_frame_t* jvm_frame_new(class_instance_t* instance, const unsigned char* method, jvm_type_t* locals, uint16_t locals_size);
void jvm_frame_free(jvm_frame_t* frame);
class_instance_t* instance_new(class_t* class);
void instance_free(class_instance_t* instance);
bool strcmp8(const uint8_t* str1, const uint8_t* str2);