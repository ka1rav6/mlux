# mlux — top-level build. Run `make help` for the target list.

ifeq ($(origin CXX),default)
CXX := clang++
endif

CLANG_FORMAT ?= clang-format
CLANG_TIDY   ?= clang-tidy
BEAR         ?= bear

PROJECT   := mlux
SRC_DIR   := src
BUILD_DIR := build
PREFIX    ?= /usr/local

VERSION := $(shell git describe --tags --always 2>/dev/null || echo 0.0.0-dev)


# Flags
# ---------------------------------------------------------------------------

MODE ?= release

SANITIZERS := -fsanitize=address,undefined

CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -I$(CURDIR)/$(SRC_DIR)
LDFLAGS  :=
LIBS     :=

ifeq ($(MODE),release)
CXXFLAGS += -O3 -DNDEBUG -flto
LDFLAGS  += -flto
else ifeq ($(MODE),debug)
CXXFLAGS += -O0 -g3 -ggdb -DDEBUG -fno-omit-frame-pointer $(SANITIZERS)
LDFLAGS  += $(SANITIZERS)
else ifeq ($(MODE),sanitize)
CXXFLAGS += -O1 -g3 -ggdb -DDEBUG -fno-omit-frame-pointer $(SANITIZERS)
LDFLAGS  += $(SANITIZERS)
else
$(error unknown MODE '$(MODE)'; use release, debug or sanitize)
endif

BUILD_OUT := $(CURDIR)/$(BUILD_DIR)/$(MODE)

export CXX CXXFLAGS LDFLAGS LIBS PROJECT BUILD_OUT


# Sources
# ---------------------------------------------------------------------------

# Everything clang-format touches. Also the trigger for regenerating
# compile_commands.json.
SOURCES := $(shell find $(SRC_DIR) -type f \
	\( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) \
	-not -path '*/third_party/*' 2>/dev/null)

# Translation units clang-tidy analyzes.
TU_SOURCES := $(filter %.cpp,$(SOURCES))

COMPILE_COMMANDS := compile_commands.json

# clang-tidy matches this against the header path as it appears in the
# diagnostic, which for our compile_commands.json is relative to src/ and
# never contains "src/". Hence ".*" plus an explicit third_party exclude.
# It has to be passed on the command line: HeaderFilterRegex in .clang-tidy
# is ignored unless clang-tidy runs with src/ as its cwd. The copy in
# .clang-tidy is for clangd and the editors.
HEADER_FILTER := .*
EXCLUDE_HEADER_FILTER := .*third_party.*

TIDY_FLAGS := -p $(CURDIR) \
              --header-filter='$(HEADER_FILTER)' \
              --exclude-header-filter='$(EXCLUDE_HEADER_FILTER)'


.PHONY: all build release debug sanitize _build test bench fmt format \
        format-check tidy tidy-fix check compile-commands install uninstall \
        clean distclean help info version


# Build
# ---------------------------------------------------------------------------

all: build
build: release

release debug sanitize:
	@$(MAKE) --no-print-directory _build MODE=$@

_build:
	@echo "  BUILD   $(MODE)/$(PROJECT)"
	@$(MAKE) --no-print-directory -C $(SRC_DIR)

test: build
	@$(MAKE) --no-print-directory -C tests

bench: build
	@$(MAKE) --no-print-directory -C bench


# compile_commands.json
# ---------------------------------------------------------------------------

# Bear can only record compilations that actually run, so build into a
# throwaway directory to force a full one without clobbering build/$(MODE).
$(COMPILE_COMMANDS): $(SOURCES) Makefile $(SRC_DIR)/Makefile
	@echo "  BEAR    $@"
	@$(BEAR) --output $@ -- $(MAKE) --no-print-directory -C $(SRC_DIR) \
		BUILD_OUT=$(CURDIR)/$(BUILD_DIR)/bear
	@rm -rf $(BUILD_DIR)/bear

compile-commands: $(COMPILE_COMMANDS)


# Formatting and static analysis
# ---------------------------------------------------------------------------

fmt format:
	@echo "  FMT     $(words $(SOURCES)) files"
	@test -z "$(strip $(SOURCES))" || $(CLANG_FORMAT) -i $(SOURCES)

format-check:
	@test -z "$(strip $(SOURCES))" || $(CLANG_FORMAT) --dry-run --Werror $(SOURCES)
	@echo "  CHECK   formatting ok"

tidy: $(COMPILE_COMMANDS)
	@echo "  TIDY    $(words $(TU_SOURCES)) translation units"
	@test -z "$(strip $(TU_SOURCES))" || $(CLANG_TIDY) $(TIDY_FLAGS) $(TU_SOURCES)

# --fix only rewrites the ranges it touched, so re-run clang-format after.
tidy-fix: $(COMPILE_COMMANDS)
	@echo "  TIDY    applying fixes"
	@test -z "$(strip $(TU_SOURCES))" || $(CLANG_TIDY) $(TIDY_FLAGS) --fix $(TU_SOURCES)
	@$(MAKE) --no-print-directory fmt

check: format-check tidy


# Install
# ---------------------------------------------------------------------------

install: release
	@echo "  INSTALL $(DESTDIR)$(PREFIX)/bin/$(PROJECT)"
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@install -m 755 $(BUILD_OUT)/$(PROJECT) $(DESTDIR)$(PREFIX)/bin/$(PROJECT)

uninstall:
	@echo "  REMOVE  $(DESTDIR)$(PREFIX)/bin/$(PROJECT)"
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(PROJECT)


# Cleaning
# ---------------------------------------------------------------------------

clean:
	@echo "  CLEAN   $(BUILD_DIR)/"
	@rm -rf $(BUILD_DIR)

distclean: clean
	@echo "  CLEAN   generated files"
	@rm -f $(COMPILE_COMMANDS)
	@rm -rf .cache Testing


# Info
# ---------------------------------------------------------------------------

version:
	@echo $(VERSION)

info:
	@echo "$(PROJECT) $(VERSION)"
	@echo "  compiler  $(CXX)"
	@echo "  mode      $(MODE)"
	@echo "  flags     $(CXXFLAGS)"
	@echo "  output    $(BUILD_OUT)/$(PROJECT)"
	@echo "  install   $(PREFIX)/bin/$(PROJECT)"
	@echo "  sources   $(words $(TU_SOURCES)) cpp, $(words $(SOURCES)) total"

help:
	@echo "mlux $(VERSION)"
	@echo
	@echo "  all / build        release build (default)"
	@echo "  release            -O3 + LTO"
	@echo "  debug              -O0 -g3 with asan/ubsan"
	@echo "  sanitize           -O1 -g3 with asan/ubsan"
	@echo "  test bench         run tests / benchmarks"
	@echo
	@echo "  fmt                clang-format in place"
	@echo "  format-check       clang-format, fail on diff"
	@echo "  tidy               clang-tidy, report only"
	@echo "  tidy-fix           clang-tidy --fix, then fmt"
	@echo "  check              format-check + tidy"
	@echo "  compile-commands   regenerate compile_commands.json"
	@echo
	@echo "  install uninstall  PREFIX=$(PREFIX)"
	@echo "  clean distclean    drop build/ ; also generated files"
	@echo "  info version help"
