#pragma once

#include "strbuf.h"
#include "type.h"

#include <stdint.h>

typedef int64_t Int;

typedef struct Value {
    Type *type;

    union {
        Int i;
        StrBuf *s;
    };
} Value;
