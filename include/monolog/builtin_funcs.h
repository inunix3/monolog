/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include "function.h"
#include "hashmap.h"
#include "type.h"

#define DECLARE_BUILTIN(_name)                                                 \
    ExprResult builtin_##_name(                                                \
        Interpreter *self, Value *args, const AstNode *node                    \
    )

DECLARE_BUILTIN(print);
DECLARE_BUILTIN(println);
DECLARE_BUILTIN(exit);
DECLARE_BUILTIN(input_int);
DECLARE_BUILTIN(input_string);
DECLARE_BUILTIN(random);
DECLARE_BUILTIN(random_range);
