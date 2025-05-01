/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <greatest.h>

/* Suites and test cases are defined in other .c files in the same directory */
SUITE(expressions);
// SUITE(invalid);
// SUITE(statements);

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(expressions);
    // RUN_SUITE(invalid);
    // RUN_SUITE(statements);

    GREATEST_MAIN_END();
}
