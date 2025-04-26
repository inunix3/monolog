#include <monolog/hashmap.h>

#include <greatest.h>

static HashMap g_map;

typedef struct TestInt {
    char buf[sizeof("TestKey") + 5];
    int value;
    bool seen;
} TestData;

static void fill_test_data(TestData *data, int n) {
    for (int i = 0; i < n; ++i) {
        data[i].value = i;
        data[i].seen = false;
        snprintf(data[i].buf, sizeof(data[i].buf), "TestKey%d", i);

        hashmap_add(&g_map, data[i].buf, &data[i].value);
    }
}

void set_up(void *udata) {
    (void)udata;

    hashmap_init(&g_map);
}

void tear_down(void *udata) {
    (void)udata;

    hashmap_deinit(&g_map);
}

TEST empty_map(void) {
    ASSERT_EQ(HASHMAP_DEFAULT_CAP, g_map.cap);
    ASSERT_EQ(0, g_map.size);
    ASSERT(g_map.buckets != NULL);

    PASS();
}

TEST add_one_value(void) {
    hashmap_add(&g_map, "TestKey", "Some value");

    ASSERT_EQ(HASHMAP_DEFAULT_CAP, g_map.cap);
    ASSERT_EQ(1, g_map.size);
    ASSERT(g_map.buckets != NULL);

    for (HashMapIter it = hashmap_iter(&g_map); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        ASSERT_STR_EQ("TestKey", it.bucket->key);
        ASSERT_STR_EQ("Some value", it.bucket->value);
    }

    PASS();
}

TEST get_one_value(void) {
    hashmap_add(&g_map, "TestKey", "Some value");

    ASSERT_EQ(HASHMAP_DEFAULT_CAP, g_map.cap);
    ASSERT_EQ(1, g_map.size);
    ASSERT(g_map.buckets != NULL);

    const char *value = hashmap_get(&g_map, "TestKey");

    ASSERT(value != NULL);
    ASSERT_STR_EQ("Some value", value);

    PASS();
}

TEST delete_one_value(void) {
    hashmap_add(&g_map, "TestKey", "Some value");

    ASSERT_EQ(HASHMAP_DEFAULT_CAP, g_map.cap);
    ASSERT_EQ(1, g_map.size);
    ASSERT(g_map.buckets != NULL);

    hashmap_remove(&g_map, "TestKey");

    ASSERT_EQ(HASHMAP_DEFAULT_CAP, g_map.cap);
    ASSERT_EQ(0, g_map.size);
    ASSERT(g_map.buckets != NULL);
    ASSERT_EQ(NULL, hashmap_get(&g_map, "TestKey"));

    PASS();
}

TEST getting_values(void) {
    TestData data[50] = { 0 };

    fill_test_data(data, 50);

    ASSERT_EQ(HASHMAP_DEFAULT_CAP, g_map.cap);
    ASSERT_EQ(50, g_map.size);
    ASSERT(g_map.buckets != NULL);

    for (int i = 0; i < 50; ++i) {
        int *value = hashmap_get(&g_map, data[i].buf);

        ASSERT(value != NULL);
        ASSERT_EQ(i, *value);
    }

    PASS();
}

TEST add_more_values_and_iteration(void) {
    TestData data[50] = { 0 };

    fill_test_data(data, 50);

    ASSERT_EQ(HASHMAP_DEFAULT_CAP, g_map.cap);
    ASSERT_EQ(50, g_map.size);
    ASSERT(g_map.buckets != NULL);

    int cnt = 0;
    for (HashMapIter it = hashmap_iter(&g_map); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        int value = *(int *) it.bucket->value;

        ASSERT(cnt < 50);
        ASSERT(0 <= value && value < 50);

        data[value].seen = true;

        ++cnt;
    }

    for (int i = 0; i < 50; ++i) {
        ASSERT(data[i].seen);
    }

    PASS();
}

TEST grow(void) {
    TestData data[HASHMAP_DEFAULT_CAP * 2] = { 0 };

    fill_test_data(data, HASHMAP_DEFAULT_CAP * 2);

    /* Hashmap doubles it's size every insertion when the load factor is >= 70% */
    ASSERT_EQ(HASHMAP_DEFAULT_CAP * 4, g_map.cap);
    ASSERT_EQ(HASHMAP_DEFAULT_CAP * 2, g_map.size);
    ASSERT(g_map.buckets != NULL);

    for (int i = 0; i < HASHMAP_DEFAULT_CAP * 2; ++i) {
        int *value = hashmap_get(&g_map, data[i].buf);

        ASSERT(value != NULL);
        ASSERT_EQ(i, *value);
    }

    PASS();
}

TEST capacity_remains_the_same_after_deleting_all(void) {
    TestData data[HASHMAP_DEFAULT_CAP * 2] = { 0 };

    fill_test_data(data, HASHMAP_DEFAULT_CAP * 2);

    for (int i = 0; i < HASHMAP_DEFAULT_CAP * 2; ++i) {
        hashmap_remove(&g_map, data[i].buf);
    }

    /* Hashmap doubles it's size every insertion when the load factor is >= 70% */
    ASSERT_EQ(HASHMAP_DEFAULT_CAP * 4, g_map.cap);
    ASSERT_EQ(0, g_map.size);
    ASSERT(g_map.buckets != NULL);

    PASS();
}

TEST capacity_remains_the_same_after_clearing(void) {
    TestData data[HASHMAP_DEFAULT_CAP * 2] = { 0 };

    fill_test_data(data, HASHMAP_DEFAULT_CAP * 2);

    hashmap_clear(&g_map);

    /* Hashmap doubles it's capacity every insertion when the load factor is >= 70% */
    ASSERT_EQ(HASHMAP_DEFAULT_CAP * 4, g_map.cap);
    ASSERT_EQ(0, g_map.size);
    ASSERT(g_map.buckets != NULL);

    PASS();
}

SUITE(hashmap) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(empty_map);
    RUN_TEST(add_one_value);
    RUN_TEST(get_one_value);
    RUN_TEST(delete_one_value);
    RUN_TEST(getting_values);
    RUN_TEST(add_more_values_and_iteration);
    RUN_TEST(grow);
    RUN_TEST(capacity_remains_the_same_after_deleting_all);
    RUN_TEST(capacity_remains_the_same_after_clearing);
}

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(hashmap);

    GREATEST_MAIN_END();
}
