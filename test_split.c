#include <stdio.h>
#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "picocom.h"
#include "split.h"

typedef struct {
    char *input;
    int rc;
    int argc;
    int index;
    char *expected;
} paramspec;

TestSuite(SplitQuoted);

ParameterizedTestParameters(SplitQuoted, test_split_quoted) {
    static paramspec params[] = {
        {"", 0, 0, 0, NULL},                        // an empty string
        {"this is a test", 0, 4, 3, "test"},        // a string with no quotes
        {"this \"is a\" test", 0, 3, 1, "is a"},    // a string with quotes
        {"    ", 0, 0, 0, NULL},                    // all whitespace
        {"this is \\a test", 0, 4, 2, "a"},         // contains an escape
        {"this is \"a test", -1, 2, 0, NULL},       // unterminated string
    };
    size_t nb_params = sizeof (params) / sizeof (paramspec);

    return cr_make_param_array(paramspec, params, nb_params);
}

ParameterizedTest(paramspec *param, SplitQuoted, test_split_quoted) {
    int argc = 0;
    char *argv[RUNCMD_ARGS_MAX + 1];
    int r;

    r = split_quoted(param->input, &argc, argv, RUNCMD_ARGS_MAX);
    /*
    fprintf(stderr, "want: argc = %d, argv[%d] = %s\n", param->argc, param->index, param->expected);
    fprintf(stderr, "have: argc = %d, argv[%d] = %s\n", argc, param->index, argv[param->index]);
    */
    cr_assert(r == param->rc);
    cr_assert(argc == param->argc);

    if (param->expected != NULL) {
        cr_assert(strcmp(argv[param->index], param->expected) == 0);
    }
}
