# ============================================================================
#  mlux — root Makefile
# ============================================================================
#  Build modes:
#    make / make all       → optimized release build
#    make debug            → debug build with symbols and sanitizers
#    make release          → optimized release build (LTO enabled)
#    make sanitize         → address + undefined behavior sanitizer build
#
#  Tags:
#    make test             → run test suite
#    make bench            → run benchmarks
#    make fmt              → format source code with clang-format
#    make install          → install binary to /usr/local/bin
#    make uninstall        → remove installed binary
#
#  Info:
#    make help             → show available targets
#    make info             → show build configuration
#    make version          → print version string
# ============================================================================

# --- Toolchain --------------------------------------------------------------
ifeq ($(origin CXX),default)
  CXX := clang++
endif
CLANGFmt  ?= clang-format

# --- Project ----------------------------------------------------------------
PROJECT   := mlux
SRC_DIR   := src
BUILD_DIR := build
PREFIX    ?= /usr/local

# --- Version (git tag fallback to 0.0.0-dev) --------------------------------
VERSION   := $(shell git describe --tags --always 2>/dev/null || echo "0.0.0-dev")

# --- Exported to sub-Makefiles ----------------------------------------------
export CXX CLANGFmt
export PROJECT SRC_DIR BUILD_DIR VERSION

# --- Source list (expanded only when a recipe needs it) ---------------------
FMT_FILES = $(shell find $(SRC_DIR) -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -not -path '*/third_party/*' 2>/dev/null)

# --- Build flags (defaults: release) ----------------------------------------
BASEFLAGS  := -std=c++20 -Wall -Wextra -Wpedantic -I$(CURDIR)/src
CXXFLAGS   := $(BASEFLAGS) -O2
LDFLAGS    :=
LIBS       :=

# --- Build mode overrides ---------------------------------------------------
MODE ?= release

ifeq ($(MODE),debug)
  CXXFLAGS := $(BASEFLAGS) -O0 -g3 -ggdb -DDEBUG \
              -fsanitize=address,undefined -fno-omit-frame-pointer
  LDFLAGS  := -fsanitize=address,undefined
else ifeq ($(MODE),release)
  CXXFLAGS := $(BASEFLAGS) -O3 -DNDEBUG -flto
  LDFLAGS  := -flto
else ifeq ($(MODE),sanitize)
  CXXFLAGS := $(BASEFLAGS) -O1 -g3 -ggdb -DDEBUG \
              -fsanitize=address,undefined -fno-omit-frame-pointer
  LDFLAGS  := -fsanitize=address,undefined
endif

export CXXFLAGS LDFLAGS LIBS

# --- Build mode directory suffix --------------------------------------------
ifeq ($(MODE),debug)
  BUILD_TYPE := debug
else ifeq ($(MODE),sanitize)
  BUILD_TYPE := sanitize
else
  BUILD_TYPE := release
endif

BUILD_OUT := $(CURDIR)/$(BUILD_DIR)/$(BUILD_TYPE)
export BUILD_OUT
export PREFIX

# --- Phony targets ----------------------------------------------------------
.PHONY: all build debug release sanitize clean distclean \
        test bench fmt format check install uninstall \
        help info version

# --- Default target ---------------------------------------------------------
all: build

# --- Build mode targets -----------------------------------------------------
build: release

release:
	@$(MAKE) --no-print-directory _build MODE=release

debug:
	@$(MAKE) --no-print-directory _build MODE=debug

sanitize:
	@$(MAKE) --no-print-directory _build MODE=sanitize

_build:
	@echo "  BUILD   $(BUILD_TYPE)/$(PROJECT) ($(shell echo $(CXXFLAGS) | tr ' ' '\n' | grep -E '^-O' | head -1))"
	@$(MAKE) --no-print-directory -C $(SRC_DIR)

# --- Test & Bench -----------------------------------------------------------
test: build
	@$(MAKE) --no-print-directory -C tests

bench: build
	@$(MAKE) --no-print-directory -C bench

# --- Format -----------------------------------------------------------------
fmt format:
	@echo "  FMT     formatting source files..."
	@files="$(FMT_FILES)"; test -n "$$files" && $(CLANGFmt) -i $$files || true
	@echo "  FMT     done."

check:
	@echo "  CHECK   verifying formatting..."
	@files="$(FMT_FILES)"; test -n "$$files" || exit 0; \
	$(CLANGFmt) --dry-run --Werror $$files

# --- Install / Uninstall ---------------------------------------------------
install: release
	@echo "  INSTALL $(PROJECT) → $(DESTDIR)$(PREFIX)/bin/$(PROJECT)"
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@install -m 755 $(BUILD_OUT)/$(PROJECT) $(DESTDIR)$(PREFIX)/bin/$(PROJECT)

uninstall:
	@echo "  REMOVE  $(DESTDIR)$(PREFIX)/bin/$(PROJECT)"
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(PROJECT)

# --- Clean ------------------------------------------------------------------
clean:
	@echo "  CLEAN   removing build artifacts..."
	@rm -rf $(BUILD_DIR)

distclean: clean
	@echo "  DISTCLEAN removing all generated files..."
	@rm -rf $(BUILD_DIR) .cache Testing

# --- Info & Help ------------------------------------------------------------
version:
	@echo $(VERSION)

info:
	@echo "($(PROJECT)) v$(VERSION)"
	@echo ""
	@echo "  Compiler  : $(CXX)"
	@echo "  Flags     : $(CXXFLAGS)"
	@echo "  Mode      : $(MODE) ($(BUILD_TYPE))"
	@echo "  Output    : $(BUILD_OUT)/$(PROJECT)"
	@echo "  Install   : $(PREFIX)/bin/$(PROJECT)"
	@echo "  Sources   : $(shell find $(SRC_DIR) -name '*.cpp' 2>/dev/null | wc -l) files"

help:
	@echo ""
	@echo "  mlux build system — v$(VERSION)"
	@echo ""
	@echo "  Build Targets:"
	@echo "    make                  Build optimized release (default)"
	@echo "    make build            Same as 'make' (optimized release)"
	@echo "    make release          Build optimized release with LTO"
	@echo "    make debug            Build debug with symbols + sanitizers"
	@echo "    make sanitize         Build with address/undefined sanitizers"
	@echo ""
	@echo "  Development:"
	@echo "    make test             Run test suite"
	@echo "    make bench            Run benchmarks"
	@echo "    make fmt              Format source with clang-format"
	@echo "    make check            Verify source formatting"
	@echo ""
	@echo "  Install:"
	@echo "    make install          Install to $(PREFIX)/bin/"
	@echo "    make uninstall        Remove from $(PREFIX)/bin/"
	@echo ""
	@echo "  Utility:"
	@echo "    make clean            Remove build artifacts"
	@echo "    make distclean        Remove all generated files"
	@echo "    make info             Show build configuration"
	@echo "    make version          Print version string"
	@echo "    make help             Show this help"
	@echo ""
