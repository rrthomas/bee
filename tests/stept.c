/* STEPT.C

    (c) Reuben Thomas 1994-2018

    Test that single_step works, and that address alignment and bounds
    checking is properly performed on EP.

*/


#include "btests.h"


int main(void)
{
    int exception = 0;

    init_beetle((CELL *)calloc(1024, 1), 256);

    NEXT;

    for (int i = 0; i < 10; i++) {
        printf("EP = %u\n", EP);
        single_step();
    }

    printf("EP should now be 60\n");
    if (EP != 60) {
        printf("Error in StepT: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    assert(exception == 0);
    printf("StepT ran OK\n");
    return 0;
}
