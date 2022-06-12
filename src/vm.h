#pragma once
#include <stdbool.h>
#include "class.h"

typedef struct {
	bool is_reference;
	union {
		int8_t byte;
		int16_t short_value;
		int32_t int_value;
		int64_t long_value;
		uint16_t char_value;
		float float_value;
		double double_value;
		bool boolean;
		uint32_t return_address;
		void* reference;
	};
} jvm_type_t;

#define GC_STAGE_MASK 3
typedef enum {
	GC_STAGE_0 = 0,
	GC_STAGE_1 = 1,
	GC_STAGE_2 = 2,
	GC_STAGE_ACCESSIBLE = 1 << 2,
	GC_STAGE_NEVER = 3
} gc_stage_t;

typedef enum {
	TYPE_CLASS_INSTANCE
} jvm_type_id_t;

typedef struct jvm_gc_type {
	gc_stage_t stage;
	jvm_type_id_t type;
	void* data;
	struct jvm_gc_type* next;
} jvm_gc_type;

typedef struct {
	struct jvm_frame_t* current_frame;
	class_t** classes;
	size_t class_count;
	jvm_gc_type* objects;
	jvm_gc_type* objects_end;
} jvm_t;

typedef struct jvm_frame_t jvm_frame_t;
jvm_type_t vm_execute(jvm_t jvm, jvm_frame_t* frame);
typedef struct class_instance_t class_instance_t;
void jvm_prepare_main_frame(jvm_t vm);
jvm_t jvm_new(class_instance_t* instance);