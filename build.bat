gcc -std=c11 -fstack-clash-protection -Wall -Wextra -Wpedantic^
    -Wno-unknown-pragmas -Wno-unused-function -Wno-padded^
    -Wno-missing-field-initializers -Wno-missing-braces -DNDEBUG -O3^
    -I third-party/isocline/include^
    -I include^
    third-party/isocline/src/isocline.c^
    src/ast.c^
    src/diagnostic.c^
    src/environment.c^
    src/function.c^
    src/hashmap.c^
    src/interp.c^
    src/lexer.c^
    src/main.c^
    src/parser.c^
    src/scope.c^
    src/semck.c^
    src/strbuf.c^
    src/type.c^
    src/utils.c^
    src/vector.c -o monolog.exe

pause
