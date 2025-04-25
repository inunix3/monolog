#include "shared.h"

TEST binary_int_int(void) {
    CHECK("115 + 94");

    PASS();
}

TEST binary_string_string(void) {
    CHECK("\"Hello\" + \", World!\"");

    PASS();
}

TEST unary_int(void) {
    CHECK("-115; +94; !0");

    PASS();
}

SUITE(valid) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(binary_int_int);
    RUN_TEST(binary_string_string);
    RUN_TEST(unary_int);
}
