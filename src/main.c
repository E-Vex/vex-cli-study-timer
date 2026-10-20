#include <stdlib.h>

#include "timer/timer.h"
#include "cli/cli.h"
#include "input/input.h"

int main(int argc, char *argv[])
{
    timer_config_t timer_config = {0};
    cli_result_t parse_result = parse_timer_args(argc, argv, &timer_config);

    switch (parse_result)
    {
    case CLI_RESULT_HELP:
        print_usage(stdout);
        return EXIT_SUCCESS;

    case CLI_RESULT_VERSION:
        print_version();
        return EXIT_SUCCESS;

    case CLI_RESULT_OK:
    {
        int result;

        vext_input_init();
        result = vext_controller(&timer_config);
        vext_input_restore();

        if (vext_input_interrupted())
        {
            int signal_number = vext_input_interrupt_signal();
            return signal_number > 0 ? 128 + signal_number : VEXT_EXIT_INTERRUPTED;
        }

        return result;
    }

    case CLI_RESULT_ERROR:
    default:
        return EXIT_FAILURE;
    }
}
