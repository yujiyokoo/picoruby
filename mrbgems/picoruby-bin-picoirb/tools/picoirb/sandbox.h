#include <vm.h>
#include <value.h>

void c_sandbox_state(mrb_vm *vm, mrb_value *v, int argc);
void c_sandbox_result(mrb_vm *vm, mrb_value *v, int argc);
void c_sandbox_resume(mrb_vm *vm, mrb_value *v, int argc);
void c_sandbox_compile(mrb_vm *vm, mrb_value *v, int argc);
void c_sandbox_exit(mrb_vm *vm, mrb_value *v, int argc);

void create_sandbox(void);

#define SANDBOX_INIT() do { \
  mrbc_class *mrbc_class_Sandbox = mrbc_define_class(0, "Sandbox", mrbc_class_object); \
  mrbc_define_method(0, mrbc_class_Sandbox, "compile", c_sandbox_compile); \
  mrbc_define_method(0, mrbc_class_Sandbox, "resume",  c_sandbox_resume);  \
  mrbc_define_method(0, mrbc_class_Sandbox, "state",   c_sandbox_state);   \
  mrbc_define_method(0, mrbc_class_Sandbox, "result",  c_sandbox_result);  \
  mrbc_define_method(0, mrbc_class_Sandbox, "exit",    c_sandbox_exit);    \
} while (0)


