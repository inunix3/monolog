gcc -std=c11 -fstack-clash-protection -Wall -Wextra -Wpedantic \
    -Wno-unknown-pragmas -Wno-unused-function -Wno-padded \
    -Wno-missing-field-initializers -Wno-missing-braces -DNDEBUG -O3 \
    -I third-party/isocline/include \
    -I include \
    third-party/isocline/src/isocline.c \
    src/*.c -o monolog

