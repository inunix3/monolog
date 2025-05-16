/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include "strbuf.h"
#include "type.h"
#include "vector.h"

#include <stdint.h>

typedef int64_t Int;
typedef struct Scope Scope;

typedef struct Value {
    Type *type;
    Scope *scope;

    union {
        Int i;
        /* Pointer to a string, allocated inside interpreter */
        StrBuf *s;

        struct {
            /* Pointer to a Vector<Value>, allocated inside interpreter */
            Vector *values; /* Vector<Value> */
        } list;

        struct {
            /* Pointer to a value, allocated inside interpreter */
            struct Value *val;
        } opt;
    };
} Value;
