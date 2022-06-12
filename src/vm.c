#include "vm.h"
#include "frame.h"
#include <stdio.h>
#include <threads.h>

void gc(jvm_t jvm) {
	fprintf(stderr, "GC begin!\n");
	if (!jvm.objects) return;

	for (jvm_gc_type* object = jvm.objects; object;) {
		if (object->stage != GC_STAGE_NEVER)
			object->stage = 0;
		object = object->next;
	}

	for (size_t i = 0; i < jvm.current_frame->sp; ++i) {
		if (jvm.current_frame->stack[i].is_reference) {
			for (jvm_gc_type* object = jvm.objects; object; object = object->next) {
				if (jvm.current_frame->stack[i].reference == object->data) {
					if (object->stage != GC_STAGE_NEVER)
						object->stage = 1;
					break;
				}
			}
		}
	}
	for (size_t i = 0; i < jvm.current_frame->locals_size; ++i) {
		if (jvm.current_frame->locals[i].is_reference) {
			for (jvm_gc_type* object = jvm.objects; object; object = object->next) {
				if (jvm.current_frame->stack[i].reference == object->data) {
					if (object->stage != GC_STAGE_NEVER)
						object->stage = 1;
					break;
				}
			}
		}
	}

	jvm_gc_type* previous = NULL;
	for (jvm_gc_type* object = jvm.objects; object;) {
		if (object->stage == 0) {
			switch (object->type) {
				case TYPE_CLASS_INSTANCE:
					instance_free(object->data);
					if (previous) previous->next = object->next;
					free(object);
					object = previous->next;
					continue;
			}
		}

		previous = object;
		object = object->next;
	}
}

void jvm_objects_add(jvm_t jvm, void* object, jvm_type_id_t type) {
	if (!jvm.objects) {
		jvm.objects = malloc(sizeof(jvm_gc_type));
		jvm.objects_end = jvm.objects;
		jvm.objects->data = object;
		jvm.objects->next = NULL;
		jvm.objects->stage = GC_STAGE_0;
		jvm.objects->type = type;
	}
	else {
		jvm.objects_end->next = malloc(sizeof(jvm_gc_type));
		jvm.objects_end = jvm.objects_end->next;
		jvm.objects_end->data = object;
		jvm.objects_end->next = NULL;
		jvm.objects_end->stage = GC_STAGE_0;
		jvm.objects_end->type = type;
	}
}

jvm_t jvm_new(class_instance_t* instance) {
	jvm_t jvm;
	jvm.current_frame = NULL;
	jvm.objects = malloc(sizeof(jvm_gc_type));
	jvm.objects_end = jvm.objects;
	jvm.objects->stage = GC_STAGE_NEVER;
	jvm.objects->type = TYPE_CLASS_INSTANCE;
	jvm.objects->data = instance;
	jvm.classes = malloc(sizeof(class_t));
	jvm.classes[jvm.class_count++] = instance->class;
	return jvm;
}

void jvm_prepare_main_frame(jvm_t vm) {
	if (!vm.objects) return;
	jvm_type_t args;
	args.reference = vm.objects->data;
	jvm_frame_t* clinit_frame = jvm_frame_new(vm.objects->data, (const uint8_t*) "<clinit>", &args, 1);
	if (!clinit_frame) return;
	vm_execute(vm, clinit_frame);
	jvm_frame_free(clinit_frame);
}

jvm_type_t vm_execute(jvm_t jvm, jvm_frame_t* frame) {
	jvm.current_frame = frame;
	while (true) {
		uint8_t op = frame->code[frame->ip++];
		switch (op) {
			case 26: // iload_0
			case 30: // lload_0
				frame->stack[frame->sp++] = frame->locals[0];
				break;
			case 27: // iload_1
			case 31: // lload_1
				frame->stack[frame->sp++] = frame->locals[1];
				break;
			case 28: // iload_2
			case 32: // lload_2
				frame->stack[frame->sp++] = frame->locals[2];
				break;
			case 29: // iload_3
			case 33: // lload_3
				frame->stack[frame->sp++] = frame->locals[3];
				break;
			case 42: // aload_0
				frame->stack[frame->sp++] = frame->locals[0];
				break;
			case 75: // astore_0
			{
				frame->locals[0] = frame->stack[--frame->sp];
				break;
			}
			case 89: // dup
			{
				uint32_t index = frame->sp - 1;
				frame->stack[frame->sp++] = frame->stack[index];
				break;
			}
			case 96: // iadd
			{
				jvm_type_t value2 = frame->stack[--frame->sp];
				frame->stack[frame->sp - 1].int_value += value2.int_value;
				break;
			}
			case 97: // ladd
			{
				jvm_type_t value2 = frame->stack[--frame->sp];
				frame->stack[frame->sp - 1].long_value += value2.long_value;
				break;
			}
			case 167: // goto
			{
				int16_t index = (int16_t) ((int16_t) frame->code[frame->ip++] << 8 | frame->code[frame->ip++]);
				frame->ip += index;
				frame->ip -= 3;
				break;
			}
			case 172: // ireturn
			case 173: // lreturn
				return frame->stack[--frame->sp];
			case 177: // return
			{
				jvm_type_t type = {};
				return type;
			}
			case 183: // invokespecial
			{
				uint16_t index = frame->code[frame->ip++] << 8 | frame->code[frame->ip++];
				constant_t* constant = &frame->instance->class->constant_pool.constants[index - 1];
				uint16_t name_and_type_index = constant->name_and_type_index;
				constant_t* name_and_type = &frame->instance->class->constant_pool.constants[name_and_type_index - 1];
				uint16_t name_index = name_and_type->name_index;
				uint16_t descriptor_index = name_and_type->descriptor_index;
				uint8_t* name = frame->instance->class->constant_pool.constants[name_index - 1].string;
				uint8_t* descriptor = frame->instance->class->constant_pool.constants[descriptor_index - 1].string;

				uint8_t* class_name = frame->instance->class->constant_pool.constants[
						frame->instance->class->constant_pool.constants[constant->class_index - 1].name_index - 1
						].string;
				// todo: provide actual java/lang/Object
				if (strcmp8(class_name, (const uint8_t*) "java/lang/Object")) {
					break;
				}
				if (strcmp8(descriptor, (const uint8_t*) "()V")) {
					jvm_type_t* args = NULL;
					uint16_t arg_count = 0;
					if (strcmp8(name, (const uint8_t*) "<init>")) {
						args = malloc(sizeof(jvm_type_t));
						// todo: args
						args[0].reference = NULL;
						args[0].is_reference = true;
						arg_count = 1;
					}
					jvm_frame_t* void_frame = jvm_frame_new(frame->stack[--frame->sp].reference, name, args, arg_count);
					if (args) free(args);
					vm_execute(jvm, void_frame);
					jvm_frame_free(void_frame);
				}
				break;
			}
			case 187: // new
			{
				uint16_t index = frame->code[frame->ip++] << 8 | frame->code[frame->ip++];
				constant_t* constant = &frame->instance->class->constant_pool.constants[index - 1];
				// Class
				if (constant->tag == 7) {
					uint8_t* name = frame->instance->class->constant_pool.constants[constant->string_index - 1].string;
					for (size_t i = 0; i < jvm.class_count; ++i) {
						if (strcmp8(jvm.classes[i]->this_class, name)) {
							class_instance_t* instance = instance_new(jvm.classes[i]);
							jvm_objects_add(jvm, instance, TYPE_CLASS_INSTANCE);
							frame->stack[frame->sp++].reference = instance;
							frame->stack[frame->sp - 1].is_reference = true;
							break;
						}
					}
				}
				break;
			}
			default:
				printf("unimplemented op %hhu\n", op);
				exit(1);
		}
		//gc(jvm);
	}
}