#pragma once

#include "strbuf.h"
#include "type.h"
#include "vector.h"

#include <stdint.h>

typedef int64_t Int;

typedef struct Value {
    Type *type;

    union {
        Int i;
        /* Pointer to a string, allocated inside interpreter */
        StrBuf *s;

        struct {
            /* Pointer to a Vector<Value>, allocated inside interpreter */
            Vector *array; /* Vector<Value> */
        } array;

        struct {
            struct Value *val;
        } opt;

        struct {
            char *ptr;
        } str_sub;
    };
} Value;
