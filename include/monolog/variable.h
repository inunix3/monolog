#pragma once

#include "type.h"
#include "value.h"

#include <stdbool.h>

typedef struct Scope Scope;

typedef struct Variable {
    Type *type;
    char *name;
    Value val;
    Scope *scope;
    bool is_param;
} Variable;
