CC       = gcc
WARN     = -Wall
SRC_DIRS = . structs parsing executing

SRCS = $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c))

# Each gets its own object directory
ifeq ($(DEBUG),1)
    CFLAGS    = $(WARN) -O0 -g
    BUILD_DIR = build/debug
else
    CFLAGS    = $(WARN) -O3
    BUILD_DIR = build/release
endif

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean interpreter_dbg
all: interpreter

interpreter: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

interpreter_dbg:
	$(MAKE) DEBUG=1 parse
	cp parse parse_dbg

# Generic rule: any .c -> .o, plus a .d file listing the headers it includes
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Pull in the generated dependency files (silently skip if missing)
-include $(DEPS)

clean:
	rm -rf build interpreter interpreter_dbg *~
