#include "../src/notify/notify.h"
#include "test_util.h"

static void test_notify_enabled_flag(void)
{
    vext_notify_set_enabled(1);
    CHECK(vext_notify_enabled() == 1);

    vext_notify_set_enabled(0);
    CHECK(vext_notify_enabled() == 0);

    vext_notify_set_enabled(42); /* any non-zero value enables */
    CHECK(vext_notify_enabled() == 1);

    vext_notify_set_enabled(1);
}

static void test_notify_null_arguments(void)
{
    vext_notify_set_enabled(1);
    vext_notify(NULL, "body");    /* must not crash */
    vext_notify("summary", NULL); /* must not crash */
    vext_notify(NULL, NULL);      /* must not crash */
}

static void test_notify_disabled_is_noop(void)
{
    vext_notify_set_enabled(0);
    vext_notify("summary", "body"); /* must not crash or send */
    vext_notify_set_enabled(1);
}

static void test_notify_available_is_cached(void)
{
    int first = vext_notify_available();

    CHECK(first == 0 || first == 1);
    CHECK(vext_notify_available() == first);
}

void test_notify(void)
{
    test_notify_enabled_flag();
    test_notify_null_arguments();
    test_notify_disabled_is_noop();
    test_notify_available_is_cached();
}
