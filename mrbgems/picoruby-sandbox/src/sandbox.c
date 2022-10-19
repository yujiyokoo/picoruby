#include <picorbc.h>
#include <mrubyc.h>
#include "sandbox.h"

//#include "sandbox_task.c"

#ifndef NODE_BOX_SIZE
#define NODE_BOX_SIZE 20
#endif

mrbc_tcb *tcb_sandbox;

static ParserState *p;
static picorbc_context *cxt;

static void
c_sandbox_state(mrb_vm *vm, mrb_value *v, int argc)
{
  SET_INT_RETURN(tcb_sandbox->state);
}

static void
c_sandbox_error(mrb_vm *vm, mrb_value *v, int argc)
{
  mrbc_vm *sandbox_vm = (mrbc_vm *)&tcb_sandbox->vm;
  if (sandbox_vm->exception.tt == MRBC_TT_NIL) {
    SET_NIL_RETURN();
  } else {
    SET_RETURN(sandbox_vm->exception);
  }
}

static void
c_sandbox_result(mrb_vm *vm, mrb_value *v, int argc)
{
  mrbc_vm *sandbox_vm = (mrbc_vm *)&tcb_sandbox->vm;
  if (sandbox_vm->regs[p->scope->sp].tt == MRBC_TT_EMPTY) {
    // fallback but FIXME
    SET_NIL_RETURN();
  } else {
    SET_RETURN(sandbox_vm->regs[p->scope->sp]);
  }
}

static void
c_sandbox_suspend(mrb_vm *vm, mrb_value *v, int argc)
{
  mrbc_suspend_task(tcb_sandbox);
  { /*
       Workaround but causes memory leak 😔
       To preserve symbol table
    */
    if (p->scope->vm_code) p->scope->vm_code = NULL;
  }
  Compiler_parserStateFree(p);
}

static void
c_sandbox_compile(mrb_vm *vm, mrb_value *v, int argc)
{
  p = Compiler_parseInitState(NODE_BOX_SIZE);
  //p->verbose = true;
  char script[255];
  sprintf(script, "_ = (%s)", (const char *)GET_STRING_ARG(1));
  StreamInterface *si = StreamInterface_new(NULL, script, STREAM_TYPE_MEMORY);
  if (!Compiler_compile(p, si, cxt)) {
    SET_FALSE_RETURN();
    Scope *upper_scope = p->scope;
    while (upper_scope->upper) upper_scope = upper_scope->upper;
    upper_scope->lvar = NULL; /* top level lvar (== cxt->sysm) should not be freed */
    Compiler_parserStateFree(p);
  } else {
    SET_TRUE_RETURN();
  }
  StreamInterface_free(si);
}

static void
c_sandbox_resume(mrb_vm *vm, mrb_value *v, int argc)
{
  mrbc_vm *sandbox_vm = (mrbc_vm *)&tcb_sandbox->vm;
  if(mrbc_load_mrb(sandbox_vm, p->scope->vm_code) != 0) {
    Compiler_parserStateFree(p);
    SET_FALSE_RETURN();
  } else {
    sandbox_vm->cur_irep = sandbox_vm->top_irep;
    sandbox_vm->inst = sandbox_vm->cur_irep->inst;
    sandbox_vm->callinfo_tail = NULL;
    sandbox_vm->target_class = mrbc_class_object;
    sandbox_vm->flag_preemption = 0;
    mrbc_resume_task(tcb_sandbox);
    SET_TRUE_RETURN();
  }
}

static void
c_sandbox_exit(mrb_vm *vm, mrb_value *v, int argc)
{
  picorbc_context_free(cxt);
}

static const uint8_t sandbox_task[] = {
0x52,0x49,0x54,0x45,0x30,0x33,0x30,0x30,0x00,0x00,0x00,0x52,0x4d,0x41,0x54,0x5a,
0x30,0x30,0x30,0x30,0x49,0x52,0x45,0x50,0x00,0x00,0x00,0x36,0x30,0x33,0x30,0x30,
0x00,0x00,0x00,0x2a,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,
0x2d,0x01,0x00,0x00,0x38,0x01,0x69,0x00,0x00,0x00,0x01,0x00,0x0c,0x73,0x75,0x73,
0x70,0x65,0x6e,0x64,0x5f,0x74,0x61,0x73,0x6b,0x00,0x45,0x4e,0x44,0x00,0x00,0x00,
0x00,0x08,
};

void
create_sandbox(void)
{
  tcb_sandbox = mrbc_create_task(sandbox_task, 0);
  tcb_sandbox->vm.flag_permanence = 1;
  cxt = picorbc_context_new();
}

void
mrbc_sandbox_init(void)
{
  mrbc_class *mrbc_class_Sandbox = mrbc_define_class(0, "Sandbox", mrbc_class_object);
  mrbc_define_method(0, mrbc_class_Sandbox, "compile", c_sandbox_compile);
  mrbc_define_method(0, mrbc_class_Sandbox, "resume",  c_sandbox_resume);
  mrbc_define_method(0, mrbc_class_Sandbox, "state",   c_sandbox_state);
  mrbc_define_method(0, mrbc_class_Sandbox, "result",  c_sandbox_result);
  mrbc_define_method(0, mrbc_class_Sandbox, "error",   c_sandbox_error);
  mrbc_define_method(0, mrbc_class_Sandbox, "suspend", c_sandbox_suspend);
  mrbc_define_method(0, mrbc_class_Sandbox, "exit",    c_sandbox_exit);
  create_sandbox();
}
