CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -DELX_SDK_VERSION=\"0.4.0-phase4\" -DELX_SDK_SECURITY_VERSION=\"0.1.0\"
CFLAGS += -Iinclude -Icompiler/include -Icompiler/src -Iruntime/include
LDFLAGS ?=
LDLIBS ?= -lm

BUILD_DIR := build
SDK_BIN := $(BUILD_DIR)/elx
SDK_LIB := $(BUILD_DIR)/libelx_sdk.a

CORE_SRCS := core/elx_core.c core/elx_error_list.c core/elx_module.c core/elx_package.c core/elx_diagnostics.c core/elx_formatter.c core/elx_linter.c core/elx_debugger.c core/elx_spec_freeze.c core/elx_compat.c core/elx_elx1.c
STD_SRCS := std/io.c std/fs.c std/process.c std/env.c std/str.c std/list.c std/map.c std/set.c std/math.c std/error.c std/net.c
NATIVE_SRCS := native/elx_native.c
SECURITY_SRCS := security/elx_security.c
BRIDGE_SRCS := core/elx_compiler_bridge.c
CLI_SRCS := cli/elx.c cli/elx_sdk.c
COMPILER_SRCS := compiler/src/ast.c compiler/src/compiler.c compiler/src/lexer.c compiler/src/parser.c compiler/src/semantic.c compiler/src/optimizer.c compiler/src/elbc.c
RUNTIME_SRCS := runtime/src/vm.c runtime/src/stack.c runtime/src/arena.c runtime/src/elbc_reader.c runtime/src/runtime.c runtime/src/elx_stdlib.c

PLATFORM_SRCS :=

ifeq ($(OS),Windows_NT)
    PLATFORM := windows
    CFLAGS += -DELX_PLATFORM_WINDOWS -D__ELVI_OS__=0 -D_GNU_SOURCE
    LDLIBS += -lws2_32
    PLATFORM_SRCS := platforms/windows/elx_win.c platforms/windows/elx_time.c
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM := linux
        CFLAGS += -DELX_PLATFORM_LINUX -D_GNU_SOURCE
        PLATFORM_SRCS := platforms/linux/elx_time.c
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macos
        CFLAGS += -DELX_PLATFORM_MACOS -D_DARWIN_C_SOURCE
        PLATFORM_SRCS := platforms/macos/elx_time.c
    endif
endif

ALL_SRCS := $(CORE_SRCS) $(STD_SRCS) $(NATIVE_SRCS) $(SECURITY_SRCS) $(BRIDGE_SRCS) $(CLI_SRCS) $(COMPILER_SRCS) $(RUNTIME_SRCS) $(PLATFORM_SRCS)
ALL_OBJS := $(ALL_SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_SRCS := tests/test_std.c
TEST_OBJS := $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_BIN := $(BUILD_DIR)/test_std

.PHONY: all test test_std clean install

all: $(SDK_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SDK_BIN): $(ALL_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(ALL_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

$(SDK_LIB): $(ALL_OBJS) | $(BUILD_DIR)
	ar rcs $@ $(filter-out $(CLI_SRCS:%.c=$(BUILD_DIR)/%.o),$(ALL_OBJS))

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(SDK_BIN)
	./$(SDK_BIN) version
	./$(SDK_BIN) doctor
	./$(SDK_BIN) init test_project
	./$(SDK_BIN) build test_project/main.elx
	./$(SDK_BIN) run test_project/main.elx
	./$(SDK_BIN) test test_project/main.elx
	rm -rf test_project
	@echo "=== SDK integration tests passed ==="

test_std: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(ALL_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_OBJS) $(ALL_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -rf $(BUILD_DIR)

install: $(SDK_BIN)
	cp $(SDK_BIN) /usr/local/bin/elx 2>/dev/null || cp $(SDK_BIN) /usr/bin/elx 2>/dev/null || echo "Install manually: cp $(SDK_BIN) to your PATH"
