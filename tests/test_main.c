#include <stdio.h>

#include "test_util.h"

int test_failures = 0;

void test_cli(void);
void test_timer(void);

int main(void)
{
    printf("Running vex-timer tests...\n");

    test_cli();
    test_timer();

    if (test_failures != 0)
    {
        fprintf(stderr, "%d test(s) failed.\n", test_failures);
        return 1;
    }

    printf("All tests passed.\n");
    return 0;
}
