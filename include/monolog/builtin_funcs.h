#pragma once

#include "function.h"

#define DECLARE_BUILTIN(_name)                                                 \
    ExprResult builtin_##_name(Interpreter *self, Value *args)

DECLARE_BUILTIN(print);
DECLARE_BUILTIN(println);
DECLARE_BUILTIN(exit);
DECLARE_BUILTIN(input_int);
DECLARE_BUILTIN(input_string);
DECLARE_BUILTIN(push);
DECLARE_BUILTIN(pop);
