##
## FEATURE SELECTION
##

# Set this to 0 to disable config file support. Building with configuration
# file support requires libconfuse (https://github.com/libconfuse/libconfuse).
FEATURE_CONFIGFILE ?= 1

# Set this to 0 to disable high baudrate support
FEATURE_HIGH_BAUD ?= 1

# Set this to 0 to disable locking via flock (and use uucp-style
# locks instead)
FEATURE_USE_FLOCK ?= 1

# Set this to 0 to disable linenoise support
FEATURE_LINENOISE ?= 1

# Set this to 0 to disable help strings (saves ~ 4-6 Kb)
FEATURE_HELP ?= 1

# Set this to 1 to enable code coverage support when building
# tests.
FEATURE_COVERAGE ?= 0

VERSION = $(git describe --long)
-include version.mk

#CC ?= gcc
CPPFLAGS = -DVERSION_STR=\"$(VERSION)\" $(EXTRA_CPPFLAGS)
CFLAGS += -Wall -g $(EXTRA_CFLAGS)

LD = $(CC)
LDFLAGS ?= -g $(EXTRA_LDFLAGS)
LDLIBS ?=

SRCS = picocom.c term.c fdio.c split.c custbaud.c termios2.c custbaud_bsd.c

## This is the maximum size (in bytes) the output (e.g. copy-paste)
## queue is allowed to grow to. Zero means unlimitted.
TTY_Q_SZ = 0
CPPFLAGS += -DTTY_Q_SZ=$(TTY_Q_SZ)

ifeq ($(FEATURE_CONFIGFILE), 1)
CPPFLAGS += -DCONFIGFILE
LDLIBS += -lconfuse
SRCS += configfile.c
endif

ifeq ($(FEATURE_HIGH_BAUD),1)
CPPFLAGS += -DHIGH_BAUD
endif

ifeq ($(FEATURE_USE_FLOCK),1)
CPPFLAGS += -DUSE_FLOCK
else
UUCP_LOCK_DIR=/var/lock
CPPFLAGS += -DUUCP_LOCK_DIR=\"$(UUCP_LOCK_DIR)\"
endif

ifeq ($(FEATURE_LINENOISE),1)
HISTFILE = .picocom_history
CPPFLAGS += -DHISTFILE=\"$(HISTFILE)\" \
	    -DLINENOISE
SRCS += linenoise-1.0/linenoise.c
endif

## Comment this in to enable (force) custom baudrate support
## even on systems not enabled by default.
#CPPFLAGS += -DUSE_CUSTOM_BAUD

## Comment this in to disable custom baudrate support
## on ALL systems (even on these enabled by default).
#CPPFLAGS += -DNO_CUSTOM_BAUD

ifeq ($(FEATURE_HELP),0)
CPPFLAGS += -DNO_HELP
endif

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
NODEPS = clean distclean realclean doc
SUFFIXES += .d
COVS = $(SRCS:.c=.gcda) $(SRCS:.c=.gcno)

TEST_SRCS = test_configfile.c test_split.c test_options.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_DEPS = $(TEST_SRCS:.c=.d)
TEST_COVS = $(TEST_SRCS:.c=.gcda) $(TEST_SRCS:.c=.gcno)

%.o: %.c %.d
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

%.d: %.c
	$(CC) $(CPPFLAGS) -MM $< -MF $@

all: picocom

check:
	echo "CPPFLAGS: $(CPPFLAGS)"

picocom : $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

doc : picocom.1.html picocom.1 picocom.1.pdf

picocom.1 : picocom.1.md
	pandoc -s -t man \
	    -Vfooter="Picocom $(VERSION)" \
	    -Vadjusting='l' \
	    -Vhyphenate='' \
	    -o $@ $<

picocom.1.html : picocom.1.md
	# modern pandoc wants --embed-resources --standalone, but
	# pandoc in github ci is too old.
	pandoc -s -t html \
	    --standalone \
	    -Vversion="v$(VERSION)" \
	    -o $@ $<

picocom.1.pdf : picocom.1.html
	htmldoc -f $@ $<

clean:
	rm -f $(OBJS) $(COVS) $(TEST_OBJS) $(TEST_COVS)
	rm -rf coverage
	rm -f *~
	rm -f \#*\#

distclean: clean
	rm -f picocom

realclean: distclean
	rm -f $(DEPS) $(TEST_DEPS)
	rm -f picocom.1
	rm -f picocom.1.html
	rm -f picocom.1.pdf
	rm -f CHANGES

ifeq (,$(findstring $(MAKECMDGOALS),$(NODEPS)))
-include $(DEPS) $(TEST_DEPS)
endif

test_picocom: CPPFLAGS+=-DTESTING
test_picocom: LDFLAGS+=-no-pie
ifeq ($(FEATURE_COVERAGE),1)
test_picocom: CFLAGS+=-fprofile-arcs -ftest-coverage
endif
test_picocom: $(TEST_OBJS) $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_OBJS) $(OBJS) $(LDLIBS) -lcriterion -lgcov

.PHONY: test
test: distclean test_picocom
	./test_picocom --verbose

ifeq ($(FEATURE_COVERAGE),1)
.PHONY: coverage
coverage: coverage/picocom.html

coverage/picocom.html: test_picocom
	mkdir -p coverage
	gcovr -r . --html --html-details -o coverage/picocom.html \
		--exclude 'test_*' --exclude 'linenoise*'
endif
