#include <monolog/vector.h>

#include <greatest.h>

static Vector g_vec;

#define NTH_VALUE(_i) (((int *) g_vec.data)[_i])

void set_up(void *udata) {
    (void) udata;

    vec_init(&g_vec, sizeof(int));
}

void tear_down(void *udata) {
    (void) udata;

    vec_deinit(&g_vec);
}

TEST empty_vec(void) {
    ASSERT_EQ(VECTOR_DEFAULT_CAP, g_vec.cap);
    ASSERT_EQ(sizeof(int), g_vec.element_size);
    ASSERT_EQ(0, g_vec.len);
    ASSERT(g_vec.data != NULL);

    PASS();
}

TEST push_one_value(void) {
    int x = 115;

    vec_push(&g_vec, &x);

    ASSERT_EQ(VECTOR_DEFAULT_CAP, g_vec.cap);
    ASSERT_EQ(1, g_vec.len);
    ASSERT(g_vec.data != NULL);

    ASSERT_EQ(115, NTH_VALUE(0));

    PASS();
}

TEST push_50_values(void) {
    for (int i = 0; i < 50; ++i) {
        vec_push(&g_vec, &i);
    }

    ASSERT_EQ(VECTOR_DEFAULT_CAP, g_vec.cap);
    ASSERT_EQ(50, g_vec.len);
    ASSERT(g_vec.data != NULL);

    for (int i = 0; i < 50; ++i) {
        ASSERT_EQ(i, NTH_VALUE(i));
    }

    PASS();
}

TEST grow(void) {
    for (int i = 0; i < VECTOR_DEFAULT_CAP * 2; ++i) {
        vec_push(&g_vec, &i);
    }

    ASSERT_EQ(VECTOR_DEFAULT_CAP * 2, g_vec.cap);
    ASSERT_EQ(VECTOR_DEFAULT_CAP * 2, g_vec.len);
    ASSERT(g_vec.data != NULL);

    for (int i = 0; i < VECTOR_DEFAULT_CAP * 2; ++i) {
        ASSERT_EQ(i, NTH_VALUE(i));
    }

    PASS();
}

TEST pop_one_value(void) {
    int x = 115;

    vec_push(&g_vec, &x);
    vec_pop(&g_vec);

    ASSERT_EQ(VECTOR_DEFAULT_CAP, g_vec.cap);
    ASSERT_EQ(0, g_vec.len);
    ASSERT(g_vec.data != NULL);

    PASS();
}

TEST pop_50_values(void) {
    for (int i = 0; i < 50; ++i) {
        vec_push(&g_vec, &i);
    }

    for (int i = 0; i < 50; ++i) {
        vec_pop(&g_vec);
    }

    ASSERT_EQ(VECTOR_DEFAULT_CAP, g_vec.cap);
    ASSERT_EQ(0, g_vec.len);
    ASSERT(g_vec.data != NULL);

    PASS();
}

TEST clear(void) {
    for (int i = 0; i < 50; ++i) {
        vec_push(&g_vec, &i);
    }

    vec_clear(&g_vec);

    ASSERT_EQ(VECTOR_DEFAULT_CAP, g_vec.cap);
    ASSERT_EQ(0, g_vec.len);
    ASSERT(g_vec.data != NULL);

    PASS();
}

TEST capacity_remains_the_same_after_popping_all(void) {
    for (int i = 0; i < VECTOR_DEFAULT_CAP * 2 + 1; ++i) {
        vec_push(&g_vec, &i);
    }

    for (int i = 0; i < VECTOR_DEFAULT_CAP * 2 + 1; ++i) {
        vec_pop(&g_vec);
    }

    /* vector doubles its capacity when it is filled */
    ASSERT_EQ(VECTOR_DEFAULT_CAP * 4, g_vec.cap);
    ASSERT_EQ(0, g_vec.len);
    ASSERT(g_vec.data != NULL);

    PASS();
}

TEST capacity_remains_the_same_after_clearing(void) {
    for (int i = 0; i < VECTOR_DEFAULT_CAP * 2; ++i) {
        vec_push(&g_vec, &i);
    }

    vec_clear(&g_vec);

    ASSERT_EQ(VECTOR_DEFAULT_CAP * 2, g_vec.cap);
    ASSERT_EQ(0, g_vec.len);
    ASSERT(g_vec.data != NULL);

    PASS();
}

SUITE(vector) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(empty_vec);
    RUN_TEST(push_one_value);
    RUN_TEST(push_50_values);
    RUN_TEST(grow);
    RUN_TEST(pop_one_value);
    RUN_TEST(pop_50_values);
    RUN_TEST(capacity_remains_the_same_after_popping_all);
    RUN_TEST(capacity_remains_the_same_after_clearing);
}

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(vector);

    GREATEST_MAIN_END();
}
