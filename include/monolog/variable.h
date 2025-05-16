/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

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
