#include <greatest.h>

SUITE(valid);
SUITE(invalid);

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(valid);
    RUN_SUITE(invalid);

    GREATEST_MAIN_END();
}
