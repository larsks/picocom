#include <criterion/criterion.h>

#include "configfile.h"

char *valid_config = "port = dummy\n"
                     "baud = 57600\n"
                     "echo = true\n"
                     "omap = {crcrlf, delbs}\n"
                     "initstring = \"ati4\\n\"\n";

extern int parse_args(int argc, char *argv[]);
extern void init_defaults(void);

int parse_config_buf(const char *buf) { return cfg_parse_buf(cfg, buf); }

static void setup(void) {
  init_defaults();
  init_config();

  fclose(stdout);
}

TestSuite(ConfigFile, .init = setup);

Test(ConfigFile, no_error_on_missing_config) {
  cr_assert(parse_config_file("does-not-exist") == 0);
}

Test(ConfigFile, error_on_invalid_option) {
  cr_assert(parse_config_buf("invalid-setting = invalid-value") != 0);
}

Test(ConfigFile, error_on_invalid_value) {
  // pass a string to an integer option
  cr_assert(parse_config_buf("baud = badvalue") != 0);

  // pass invalid string to enum option
  cr_assert(parse_config_buf("dtr = badvalue") != 0);

  // pass invalid string to boolean option
  cr_assert(parse_config_buf("noreset = badvalue") != 0);

  // pass string to list option
  cr_assert(parse_config_buf("omap = badvalue") != 0);
}

Test(ConfigFile, no_error_on_valid_config) {
  cr_assert(parse_config_buf(valid_config) == 0);
}

Test(ConfigFile, setting_config_sets_option) {
  char *args[] = {"picocom"};

  cr_assert(parse_config_buf(valid_config) == 0);
  cr_assert(parse_args(1, args) == 0);
  cr_assert(opts.port);
  cr_assert(strcmp(opts.port, "dummy") == 0);
  cr_assert(opts.baud == 57600);
  cr_assert(opts.lecho);
  cr_assert(opts.omap == (M_CRCRLF | M_DELBS));
  cr_assert(strcmp(opts.initstring, "ati4\n") == 0);
}

Test(ConfigFile, read_port_from_config) {
  char *args[] = {"picocom"};

  cr_assert(parse_args(1, args) != 0);
  cr_assert(parse_config_buf(valid_config) == 0);
  cr_assert(parse_args(1, args) == 0);
}

Test(ConfigFile, default_configfile_respects_xdg_home) {
    setenv("XDG_CONFIG_HOME", "/test", 1);
    cr_assert(strcmp(default_config_file(), "/test/picocom/picocom.conf") == 0);
}

Test(ConfigFile, default_configfile_respects_home) {
    setenv("HOME", "/test", 1);
    unsetenv("XDG_CONFIG_HOME");
    cr_assert(strcmp(default_config_file(), "/test/.config/picocom/picocom.conf") == 0);
}

Test(ConfigFile, default_configfile_returns_null_if_no_home) {
    unsetenv("HOME");
    unsetenv("XDG_CONFIG_HOME");
    cr_assert(default_config_file() == NULL);
}
