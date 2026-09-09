# mlux

A fully customizable terminal multiplexer with the ability to save sessions and restore them even after reboot (on linux).

---

## Building

mlux is built with GNU Make (recursive Makefiles). You only ever need to run `make` from the project's top-level directory — it handles the rest automatically.

### Prerequisites

- A C++20 compiler (`clang++` is used by default)
- GNU Make
- `clang-format` (for the `make fmt` and `make check` commands)

### Quick start

```sh
make            # build the optimized release binary
./build/release/mlux
```

---

## Build commands

| Command | What it does |
| ------- | ------------ |
| `make` | Build the **optimized release** binary. The default command. |
| `make build` | Same as `make` — an alias for the optimized release build. |
| `make release` | Release build with **LTO (Link-Time Optimization)** and `-O3`. |
| `make debug` | Debug build: no optimization, full debug symbols, plus **AddressSanitizer + UndefinedBehaviorSanitizer**. Best for finding bugs while developing. |
| `make sanitize` | Sanitizer build: `-O1`, symbols, Address + Undefined Behavior sanitizers. Good for running test suites under sanitizers. |
| `make clean` | Delete all build output (`build/`). |
| `make distclean` | Like `clean`, plus any other generated files/caches. |

### How the build works

1. `make` reads the **top-level Makefile** and runs one of the build modes.
2. The build mode changes the compiler flags (`-O3` for release, sanitizers for debug, etc.) and picks an output folder: `build/release`, `build/debug`, or `build/sanitize`.
3. The top-level Makefile delegates to `src/Makefile`, which:
   - finds every `.cpp` file under `src/` (recursively),
   - compiles each one into a `.o` object file **in a mirror of the folder structure** under `build/<mode>/`,
   - and links them all into the final binary `build/<mode>/mlux`.
4. Object files are reused — running `make` again only recompiles files that actually changed.

Since the object files are separated by mode, you can build all three modes side by side without them overwriting each other.

---

## Testing & benchmarks

| Command | What it does |
| ------- | ------------ |
| `make test` | Run the test suite. |
| `make bench` | Run the benchmarks. |

Both delegate to their own Makefiles (`tests/Makefile` and `bench/Makefile`). Tests and benchmarks don't exist yet — they print a friendly message until you add files.

---

## Formatting

| Command | What it does |
| ------- | ------------ |
| `make fmt` | Reformat all source files with `clang-format` (using the `.clang-format` config). |
| `make check` | Check whether the code is already formatted, without changing anything. Fails if any file needs formatting. |

---

## Install

| Command | What it does |
| ------- | ------------ |
| `make install` | Build the release binary, then copy it to `/usr/local/bin/mlux`. |
| `make uninstall` | Remove the installed `/usr/local/bin/mlux`. |

If you want a different install location, override `PREFIX`, for example:

```sh
make install PREFIX=~/.local
```

---

## Info commands

| Command | What it does |
| ------- | ------------ |
| `make info` | Show the current build settings: compiler, flags, mode, output path, install path, and how many source files are being compiled. |
| `make version` | Print the version string (derived from the latest git tag, falling back to the commit hash until a tag exists). |
| `make help` | Show the full list of available commands. |

---

## Project layout

```
mlux/
├── Makefile          # top-level: build modes, install, fmt, info, etc.
├── src/
│   └── Makefile      # finds sources and compiles/links the binary
├── tests/
│   └── Makefile      # test suite (placeholder)
├── bench/
│   └── Makefile      # benchmarks (placeholder)
├── build/            # generated automatically — don't edit
├── include/          # public headers
├── docs/             # design documentation
└── README.md
```
