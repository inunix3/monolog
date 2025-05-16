/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

typedef int (*CliCommandFn)(int argc, char **argv);

typedef struct CliCommand {
    const char *name;
    int args;
    CliCommandFn fn;
} CliCommand;

int cmd_run(int argc, char **argv);
int cmd_scan(int argc, char **argv);
int cmd_parse(int argc, char **argv);
int cmd_repl(int argc, char **argv);
