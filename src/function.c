/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/function.h>

#include <stddef.h>
#include <stdlib.h>

void fn_deinit(Function *self) {
    FnParam *params = self->params.data;
    for (size_t i = 0; i < self->params.len; ++i) {
        FnParam *param = &params[i];

        param->type = NULL;

        free(param->name);
        param->name = NULL;
    }

    vec_deinit(&self->params);

    if (self->free_body) {
        astnode_destroy(self->body);
        self->body = NULL;
    }

    free(self->name);
    self->name = NULL;
}
