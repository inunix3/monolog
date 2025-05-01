/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/interp.h>
#include <monolog/lexer.h>
#include <monolog/parser.h>
#include <monolog/semck.h>
#include <monolog/utils.h>
#include <monolog/vector.h>

#include <isocline.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
highlighter(ic_highlight_env_t *henv, const char *input, void *arg) {
    (void)arg;

    static const char *keywords[] = {"break", "continue", "return",
                                     "print", "println",  NULL};
    static const char *controls[] = {"if", "else", "while", "for", NULL};
    static const char *types[] = {"int", "string", "void", NULL};

    long len = (long)strlen(input);

    for (long i = 0; i < len; ++i) {
        switch (input[i]) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case ',':
        case ';':
        case '(':
        case ')':
        case ']':
        case '[':
        case '{':
        case '}':
        case '+':
        case '-':
        case '*':
        case '%':
        case '!':
        case '&':
        case '|':
        case '<':
        case '>':
        case '=':
        case '?':
        case '#':
        case '$':
            continue;
        default:
            break;
        }

        if (input[i] == '/' && i < len - 1 && input[i + 1] != '/') {
            continue;
        }

        long tok_len = 0;

        if ((tok_len =
                 ic_match_any_token(input, i, &ic_char_is_idletter, keywords)) >
            0) {
            ic_highlight(henv, i, tok_len, "keyword");
        } else if ((tok_len = ic_match_any_token(
                        input, i, &ic_char_is_idletter, controls
                    )) > 0) {
            ic_highlight(henv, i, tok_len, "plum");
        } else if ((tok_len = ic_match_any_token(
                        input, i, &ic_char_is_idletter, types
                    )) > 0) {
            ic_highlight(henv, i, tok_len, "type");
        } else if ((tok_len = ic_is_token(input, i, &ic_char_is_digit)) > 0) {
            ic_highlight(henv, i, tok_len, "number");
        } else if (ic_starts_with(input + i, "//")) {
            tok_len = 2;

            while (i + tok_len < len && input[i + tok_len] != '\n') {
                tok_len++;
            }

            ic_highlight(henv, i, tok_len, "comment");
        } else {
            ic_highlight(henv, i, 1, NULL);

            tok_len = 1;
        }

        i += tok_len;
    }
}

static void cmd_tokenize(char *buf, size_t size) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(buf, size, &tokens);

    for (size_t i = 0; i < tokens.len; ++i) {
        const Token *toks = tokens.data;
        const Token *tok = &toks[i];

        printf("Token %zu:\n", i + 1);
        printf("  kind: %s (%d)\n", token_kind_to_str(tok->kind), tok->kind);
        printf("  len: %zu\n", tok->len);
        printf("  src excerpt: '%.*s'\n", (int)tok->len, tok->src);
    }

    vec_deinit(&tokens);
}

static void cmd_parse(char *buf, size_t size) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(buf, size, &tokens);

    Parser parser = parser_new(tokens.data, tokens.len);
    parser.log_errors = true;

    Ast ast = parser_parse(&parser);
    ast_dump(&ast, stdout);

    if (!parser.error_state) {
        SemChecker semck;
        semck_init(&semck);
        semck_check(&semck, &ast);

        DiagnosticMessage *dmsgs = semck.dmsgs.data;

        for (size_t i = 0; i < semck.dmsgs.len; ++i) {
            printf("error: %s\n", dmsg_to_str(&dmsgs[i]));
        }

        semck_deinit(&semck);
    }

    vec_deinit(&tokens);
}

static int cmd_run(char *buf, size_t size) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(buf, size, &tokens);

    Parser parser = parser_new(tokens.data, tokens.len);
    parser.log_errors = true;

    Ast ast = parser_parse(&parser);

    bool has_error = parser.error_state;
    if (!has_error) {
        SemChecker semck;
        semck_init(&semck);
        has_error = !semck_check(&semck, &ast);

        DiagnosticMessage *dmsgs = semck.dmsgs.data;

        for (size_t i = 0; i < semck.dmsgs.len; ++i) {
            printf("error: %s\n", dmsg_to_str(&dmsgs[i]));
        }

        semck_deinit(&semck);
    }

    int exit_code = -1;

    if (!has_error) {
        Interpreter interp;
        interp_init(&interp, &ast);
        interp.log_errors = true;

        exit_code = interp_walk(&interp);
    }

    ast_destroy(&ast);
    vec_deinit(&tokens);

    return exit_code;
}

static void print_help(void) {
    printf("usage: monolog COMMAND FILENAME\n"
           "\n"
           "COMMAND can be one of:\n"
           "  tokenize - print tokens after lexing and exit\n"
           "  parse - print the AST after parsing and exit\n"
           "  help - print this message and exit\n");
}

static void cmd_repl(void) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    ic_set_default_highlighter(highlighter, NULL);
    ic_set_history(".monologhist", -1); /* -1 for default 200 entries */

    char *input;
    while ((input = ic_readline(""))) {
        bool stop = strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0;

        if (!stop) {
            lexer_lex(input, strlen(input), &tokens);

            Parser parser = parser_new(tokens.data, tokens.len);
            parser.log_errors = true;
            Ast ast = parser_parse(&parser);

            bool has_error = parser.error_state;

            if (!has_error) {
                SemChecker semck;
                semck_init(&semck);
                has_error = !semck_check(&semck, &ast);

                DiagnosticMessage *dmsgs = semck.dmsgs.data;

                for (size_t i = 0; i < semck.dmsgs.len; ++i) {
                    printf("error: %s\n", dmsg_to_str(&dmsgs[i]));
                }

                semck_deinit(&semck);
            }

            if (!has_error) {
                Interpreter interp;
                interp_init(&interp, &ast);
                interp.log_errors = true;

                interp_walk(&interp);

                if (interp.halt) {
                    exit(interp.exit_code);
                }
            }

            ast_destroy(&ast);
        }

        free(input);
        vec_clear(&tokens);

        if (stop) {
            break;
        }
    }

    vec_deinit(&tokens);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_help();

        return EXIT_FAILURE;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "repl") == 0) {
        cmd_repl();

        return EXIT_SUCCESS;
    }

    const char *filename = argv[2];

    char *input = read_file(filename);
    size_t size = strlen(input);

    if (strcmp(cmd, "tokenize") == 0) {
        cmd_tokenize(input, size);
    } else if (strcmp(cmd, "parse") == 0) {
        cmd_parse(input, size);
    } else if (strcmp(cmd, "run") == 0) {
        int exit_code = cmd_run(input, size);
        free(input);

        return exit_code;
    } else {
        fprintf(stderr, "bad command\n");
    }

    free(input);

    return EXIT_SUCCESS;
}
