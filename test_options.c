#include <stdio.h>
#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "picocom.h"
#include "configfile.h"

extern int parse_args(int argc, char *argv[]);
extern void init_defaults(void);

static void setup(void) {
  init_config();
  init_defaults();
}

struct int_opt_int_val_paramspec {
    char *option;
    int argument;
    int *value;
};

struct str_opt_int_val_paramspec {
    char *option;
    char *argument;
    int want;
    int *value;
};

struct str_opt_str_val_paramspec {
    char *option;
    char *argument;
    char **value;
};

TestSuite(Options, .init = setup);

ParameterizedTestParameters(Options, test_int_option) {
    static struct int_opt_int_val_paramspec params[] = {
        {"-b", 57600, &opts.baud},
        {"--baud", 57600, &opts.baud},
        {"-d", 5, &opts.databits},
        {"--databits", 5, &opts.databits},
        {"-p", 2, &opts.stopbits},
        {"--stopbits", 2, &opts.stopbits},
    };
    size_t nb_params = sizeof (params) / sizeof (struct int_opt_int_val_paramspec);
    return cr_make_param_array(struct int_opt_int_val_paramspec, params, nb_params);
}

ParameterizedTest(struct int_opt_int_val_paramspec *param, Options, test_int_option) {
        char value[20];
        snprintf(value, 20, "%d", param->argument);
        char *args[] = {"picocom", param->option, value, "dummy"};
        cr_assert(parse_args(4, args) == 0);
        cr_assert(*(param->value) == param->argument);
}

ParameterizedTestParameters(Options, test_flag_option_int_value) {
    static struct int_opt_int_val_paramspec params[] = {
        {"-n", 0, (int *)&opts.escape},
        {"--no-escape", 0, (int *)&opts.escape},
        {"-c", 1, &opts.lecho},
        {"--echo", 1, &opts.lecho},
        {"-i", 1, &opts.noinit},
        {"--noinit", 1, &opts.noinit},
        {"-r", 1, &opts.noreset},
        {"--noreset", 1, &opts.noreset},
        {"-u", 1, &opts.hangup},
        {"--hangup", 1, &opts.hangup},
        {"-l", 1, &opts.nolock},
        {"--nolock", 1, &opts.nolock},
        {"-X", 1, &opts.exit},
        {"--exit", 1, &opts.exit},
        {"-q", 1, &opts.quiet},
        {"--quiet", 1, &opts.quiet},
        {"--excl", 1, &opts.excl},
        {"--lower-rts", RTS_DTR_LOWER, &opts.raise_lower_rts},
        {"--raise-rts", RTS_DTR_RAISE, &opts.raise_lower_rts},
        {"--lower-dtr", RTS_DTR_LOWER, &opts.raise_lower_dtr},
        {"--raise-dtr", RTS_DTR_RAISE, &opts.raise_lower_dtr},
    };
    size_t nb_params = sizeof (params) / sizeof (struct int_opt_int_val_paramspec);
    return cr_make_param_array(struct int_opt_int_val_paramspec, params, nb_params);
}

ParameterizedTest(struct int_opt_int_val_paramspec *param, Options, test_flag_option_int_value) {
        char *args[] = {"picocom", param->option, "dummy"};
        cr_assert(parse_args(3, args) == 0);
        cr_assert(*(param->value) == param->argument);
}

ParameterizedTestParameters(Options, test_str_option_int_value) {
    static struct str_opt_int_val_paramspec params[] = {
        {"-f", "x", FC_XONXOFF, (int *)&opts.flow},
        {"-f", "h", FC_RTSCTS, (int *)&opts.flow},
        {"-f", "n", FC_NONE, (int *)&opts.flow},
        {"-f", "soft", FC_XONXOFF, (int *)&opts.flow},
        {"-f", "hard", FC_RTSCTS, (int *)&opts.flow},
        {"-f", "none", FC_NONE, (int *)&opts.flow},
        {"-y", "o", P_ODD, (int *)&opts.parity},
        {"-y", "e", P_EVEN, (int *)&opts.parity},
        {"-y", "n", P_NONE, (int *)&opts.parity},
        {"-y", "odd", P_ODD, (int *)&opts.parity},
        {"-y", "even", P_EVEN, (int *)&opts.parity},
        {"-y", "none", P_NONE, (int *)&opts.parity},
        {"--imap", "crlf,delbs", M_CRLF|M_DELBS, &opts.imap},
        {"--omap", "crlf,delbs", M_CRLF|M_DELBS, &opts.omap},
        {"--emap", "crlf,delbs", M_CRLF|M_DELBS, &opts.emap},
        {"--rts", "none", RTS_DTR_NONE, &opts.raise_lower_rts},
        {"--rts", "raise", RTS_DTR_RAISE, &opts.raise_lower_rts},
        {"--rts", "lower", RTS_DTR_LOWER, &opts.raise_lower_rts},
        {"--dtr", "none", RTS_DTR_NONE, &opts.raise_lower_dtr},
        {"--dtr", "raise", RTS_DTR_RAISE, &opts.raise_lower_dtr},
        {"--dtr", "lower", RTS_DTR_LOWER, &opts.raise_lower_dtr},
    };
    size_t nb_params = sizeof (params) / sizeof (struct str_opt_int_val_paramspec);
    return cr_make_param_array(struct str_opt_int_val_paramspec, params, nb_params);
}

ParameterizedTest(struct str_opt_int_val_paramspec *param, Options, test_str_option_int_value) {
        char *args[] = {"picocom", param->option, NULL, "dummy"};
        char *tmp;

        // This dance is necessary because options processing assumes that
        // optarg is writable.
        tmp = malloc(strlen(param->argument));
        strcpy(tmp, param->argument);
        args[2] = tmp;
        cr_assert(parse_args(4, args) == 0);
        free(tmp);
        cr_assert(*(param->value) == param->want);
}

ParameterizedTestParameters(Options, test_str_option_str_value) {
    static struct str_opt_str_val_paramspec params[] = {
        {"--send-cmd", "test", &opts.send_cmd},
        {"--receive-cmd", "test", &opts.receive_cmd},
        {"--initstring", "test", &opts.initstring},
        {"--logfile", "test", &opts.log_filename},
    };
    size_t nb_params = sizeof (params) / sizeof (struct str_opt_str_val_paramspec);
    return cr_make_param_array(struct str_opt_str_val_paramspec, params, nb_params);
}

ParameterizedTest(struct str_opt_str_val_paramspec *param, Options, test_str_option_str_value) {
        char *args[] = {"picocom", param->option, param->argument, "dummy"};
        cr_assert(parse_args(4, args) == 0);
        cr_assert(strcmp(*(param->value), param->argument) == 0);
}
