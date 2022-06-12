#include <string.h>
#include "vm.h"
#include "frame.h"

bool strcmp8(const uint8_t* str1, const uint8_t* str2) {
	for (; *str1 && *str2; ++str1, ++str2) if (*str1 != *str2) return false;
	if (!*str1 && !*str2) return true;
	return false;
}

class_instance_t* instance_new(class_t* class) {
	class_instance_t* instance = malloc(sizeof(class_instance_t));
	instance->class = class;
	instance->field_count = class->field_count;
	instance->fields = NULL;
	if (instance->field_count > 0) {
		instance->fields = malloc(class->field_count * sizeof(field_t));
		memcpy(instance->fields, class->fields, class->field_count * sizeof(field_t));
	}
	return instance;
}
void instance_free(class_instance_t* instance) {
	if (instance == NULL) return;
	if (instance->fields)
		free(instance->fields);
	free(instance);
}

jvm_frame_t* jvm_frame_new(class_instance_t* instance, const unsigned char* method, jvm_type_t* locals, uint16_t locals_size) {
	for (uint16_t i = 0; i < instance->class->method_count; ++i) {
		field_t* current_method =  instance->class->methods + i;
		if (strcmp8(current_method->name, method)) {
			for (uint16_t a = 0; a < current_method->attribute_count; ++a) {
				attribute_t* attribute = current_method->attributes + a;
				uint8_t name[] = "Code";
				if (strcmp8(attribute->name, name)) {
					jvm_frame_t* frame = malloc(sizeof(jvm_frame_t));
					frame->instance = instance;
					frame->ip = 0;
					frame->sp = 0;

					uint8_t* data = attribute->data;
					uint16_t max_stack = BSWAP16(*(uint16_t*) data);
					data += 2;
					uint16_t max_locals = BSWAP16(*(uint16_t*) data);
					data += 2;
					uint32_t code_length = BSWAP32(*(uint32_t*) data);
					data += 4;
					frame->code = malloc(code_length);
					memcpy(frame->code, data, code_length);
					data += code_length;

					uint16_t exception_table_length = BSWAP16(*(uint16_t*) data);
					data += 2;
					frame->exception_handler_count = exception_table_length;
					frame->exception_handlers = NULL;
					if (exception_table_length > 0) {
						frame->exception_handlers = malloc(exception_table_length * sizeof(jvm_exception_handler_t));
					}
					for (uint16_t e = 0; e < exception_table_length; ++e) {
						uint16_t start_pc = BSWAP16(*(uint16_t*) data);
						data += 2;
						uint16_t end_pc = BSWAP16(*(uint16_t*) data);
						data += 2;
						uint16_t handler_pc = BSWAP16(*(uint16_t*) data);
						data += 2;
						uint16_t catch_type = BSWAP16(*(uint16_t*) data);
						data += 2;
						frame->exception_handlers[e].start = start_pc;
						frame->exception_handlers[e].end = end_pc;
						frame->exception_handlers[e].handler = handler_pc;
						frame->exception_handlers[e].type = catch_type;
					}

					frame->stack = NULL;
					if (max_stack > 0) {
						frame->stack = malloc(max_stack * sizeof(jvm_type_t));
					}
					frame->sp = 0;
					frame->locals_size = max_locals;
					frame->locals = NULL;
					if (max_locals > 0) {
						frame->locals = malloc(max_locals * sizeof(jvm_type_t));
					}
					for (uint16_t l = 0; l < locals_size && l < max_locals; ++l) {
						frame->locals[l] = locals[l];
					}

					return frame;
				}
			}
		}
	}
	return NULL;
}

void jvm_frame_free(jvm_frame_t* frame) {
	if (frame == NULL) return;
	if (frame->locals != NULL)
		free(frame->locals);
	if (frame->stack != NULL)
		free(frame->stack);
	if (frame->exception_handlers != NULL)
		free(frame->exception_handlers);
	if (frame->code != NULL)
		free(frame->code);
	free(frame);
}