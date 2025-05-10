#pragma once

#include "function.h"
#include "hashmap.h"
#include "type.h"

#define DECLARE_BUILTIN(_name)                                                 \
    ExprResult builtin_##_name(Interpreter *self, Value *args)

DECLARE_BUILTIN(print);
DECLARE_BUILTIN(println);
DECLARE_BUILTIN(exit);
DECLARE_BUILTIN(input_int);
DECLARE_BUILTIN(input_string);
