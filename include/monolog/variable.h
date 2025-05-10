#pragma once

#include "type.h"
#include "value.h"

#include <stdbool.h>

typedef struct Variable {
    Type *type;
    char *name;
    Value val;
    bool is_param;
} Variable;
