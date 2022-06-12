#include <stdio.h>
#include "class.h"
#include "vm.h"
#include "frame.h"

int main() {
	class_t* class = load_class("../tests/Add.class");
	class_instance_t* instance = instance_new(class);
	jvm_t jvm = jvm_new(instance);
	jvm_prepare_main_frame(jvm);

	jvm_type_t* args = malloc(2 * sizeof(jvm_type_t));
	args[0].int_value = 2;
	args[0].is_reference = false;
	args[1].int_value = 3;
	args[1].is_reference = false;
	//jvm_frame_t* frame = jvm_frame_new(instance, (const uint8_t*) "add", args, 2);
	jvm_frame_t* frame = jvm_frame_new(instance, (const uint8_t*) "gcTest", NULL, 0);
	if (!frame) {
		printf("method was not found\n");
		exit(1);
	}

	jvm_type_t result = vm_execute(jvm, frame);
	printf("%d\n", result.int_value);

	jvm_frame_free(frame);
	free(args);
	instance_free(instance);
	class_delete(class);
	return 0;
}
