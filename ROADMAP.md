# mlux — Roadmap (Checkpoint Format)

*(Stack: C++ with plain Makefiles — no Rust, no Cargo.)*

This roadmap is broken into **phases**, and every phase is broken into **checkpoints** — small, concrete, buildable steps in the order you should actually do them. Check them off as you go. If you're wondering "where do I even start," the answer is always: **Phase 0, Checkpoint 0.1.**

Rule for the whole roadmap: **build the boring plumbing first, prove it with real daily use, save CRIU for last** — it's the hardest and riskiest part, so everything before it exists partly to make sure you don't have to debug ten new things at once when you get there.

---

## Phase −1 (optional but recommended): Feasibility spike

Do this *before* committing real time to Phase 0. It's cheap, and it tells you early whether the scariest part of the whole project (CRIU + PTYs) is even going to behave the way the design assumes. This can be done with the `criu` command-line tool directly — you don't need any of your own code yet.

- [ ] **−1.1** Install CRIU on your machine and confirm `criu check` passes (this tells you if your kernel/permissions support it at all).
- [ ] **−1.2** Checkpoint a plain `bash` process with `criu dump --shell-job -t <pid> -D <dir>` and look at what image files it produces.
- [ ] **−1.3** Restore that same dump with `criu restore --shell-job -D <dir>` in the same terminal session and confirm the shell comes back alive.
- [ ] **−1.4** Now the real test: restore that dump but redirect it onto a **newly created PTY** instead of the original one (this is the exact trick the whole CRIU integration depends on). Confirm it lands on the new terminal, not the old one.
- [ ] **−1.5** Reboot your machine, then attempt the restore again from the same saved image directory. Note exactly what breaks (PID conflict? path issue? something else?) — this tells you what Phase 4 actually needs to solve first.

**Done when:** you've personally seen a CRIU-restored shell land on a brand-new PTY at least once, even from raw `criu` CLI commands. If this doesn't work at all on your machine/kernel, better to know now than after months of building the rest.

---

## Phase 0 — Foundations

**Goal: a working multiplexer, even if it's ugly and hardcoded.**

- [ ] **0.1** Set up the project skeleton: a root directory with subfolders (`daemon/`, `client/`, `common/`), a Makefile in each that builds an object/library, and a root Makefile that recurses into all of them.
- [ ] **0.2** Open a single PTY and spawn a shell inside it using `forkpty()` (from `<pty.h>`); read its raw output with `read()` and print it straight to your own terminal (no parsing yet) — this proves the fd plumbing works at all.
- [ ] **0.3** Forward your own keystrokes into that PTY's master fd with `write()`, so you have a crude but real interactive shell.
- [ ] **0.4** Put your own terminal into raw mode using `termios` (`tcgetattr`/`tcsetattr`, clearing `ICANON`/`ECHO`) for step 0.3 to actually feel usable — remember to restore the original terminal settings on exit.
- [ ] **0.5** Bring in **libvterm** and build a `Grid` class (rows × columns of styled characters + cursor position) driven by libvterm's callbacks; feed the PTY's output through `vterm_input_write()` instead of printing raw bytes.
- [ ] **0.6** Render the `Grid` to your terminal (redraw on change) instead of printing raw bytes — this is your first real "terminal emulator" moment.
- [ ] **0.7** Handle terminal resize: catch `SIGWINCH` on your outer terminal, resize the PTY with `ioctl(fd, TIOCSWINSZ, ...)`, and resize your `Grid`/libvterm state to match.
- [ ] **0.8** Split this single-process prototype into a real **daemon + client**: move the PTY and `Grid` logic into a daemon process; the client only forwards keys in and draws whatever the daemon sends back, over an `AF_UNIX` socket (`socket()`/`bind()`/`connect()`).
- [ ] **0.9** Implement attach/detach: closing the client should NOT kill the daemon or the shell inside it; running the client again should reattach to the same daemon (by connecting to its known socket path) and show the same screen.
- [ ] **0.10** Build the core data model in C++: `Session` → `Window` → `Pane` classes, where panes inside a window are stored as a splittable tree (a recursive `SplitNode` holding either a `Pane` or two child nodes), not just a flat list.
- [ ] **0.11** Implement pane splitting (horizontal and vertical) and draw simple borders between panes.
- [ ] **0.12** Implement resizing panes when a split happens (proportional resize of the tree), and re-resizing correctly when the outer terminal itself resizes.
- [ ] **0.13** Implement pane focus/navigation — switching which pane receives your keystrokes.
- [ ] **0.14** Implement multiple windows (tabs) inside a session, and switching between them.
- [ ] **0.15** Wire up a minimal, hardcoded keybinding set just to drive all of the above: prefix key, split horizontal, split vertical, move focus, new window, next/previous window, detach.
- [ ] **0.16** Use it for real, daily work for at least a few days (running `vim`, a build tool, `htop`, etc. inside it) and fix whatever breaks. Also run a first pass with AddressSanitizer/UndefinedBehaviorSanitizer builds now, while the codebase is still small — it's much cheaper to fix memory bugs early than after Phase 3.

**Done when:** you could use this instead of tmux for ordinary daily work and not immediately hit a wall.

---

## Phase 1 — Lua configuration and real keybindings

**Goal: replace tmux as your daily config, no CRIU yet.**

- [ ] **1.1** Link against `liblua5.4` and add **sol2** (a single header) to the project; get a bare Lua state running at daemon startup (even just `lua.script("print('hello')")`).
- [ ] **1.2** Design the first slice of the typed API you'll expose to Lua (e.g. `mux.map`, `mux.options`) and bind it using sol2's `set_function`/`new_usertype`.
- [ ] **1.3** Load `~/.config/mlux/init.lua` at daemon startup and run it.
- [ ] **1.4** Move your Phase 0 hardcoded keybindings into Lua, driven by `mux.map(key, action)` calls in `init.lua`.
- [ ] **1.5** Move color/appearance options into Lua-driven `mux.options.*` settings (status bar colors, border colors, etc.).
- [ ] **1.6** Implement a config-reload command that re-runs `init.lua` (in a fresh sol2 state, then swaps it in) and applies changes live, without restarting the daemon or dropping any panes.
- [ ] **1.7** Build the status bar rendering pipeline: a bar made of an ordered list of "segments," each producing text + a color.
- [ ] **1.8** Expose a `mux.status.add({ render = ..., update_on = {...} })` API to Lua via sol2 so segments can be defined in config.
- [ ] **1.9** Ship a basic default status bar using that API: session name, window list, clock.
- [ ] **1.10** Implement copy mode: entering/exiting a distinct mode, and cursor navigation through the pane's scrollback while in it.
- [ ] **1.11** Implement text selection and yanking to an internal buffer inside copy mode.
- [ ] **1.12** Hook that internal buffer up to the real system clipboard by shelling out to `wl-copy`/`xclip` (via `fork()`/`exec()`) depending on the session type.
- [ ] **1.13** Implement per-project config: on entering/creating a session in a given directory, look for a `.mlux.lua` file there and merge it over the global config.
- [ ] **1.14** Dogfood this as your actual daily driver for at least a week and fix whatever friction shows up.

**Done when:** you'd genuinely be comfortable using this instead of tmux day-to-day, purely on config and feel — still zero CRIU features.

---

## Phase 2 — Event system and plugin API v1

**Goal: an outside contributor could build a real plugin using only your docs.**

- [ ] **2.1** Design and implement an internal event bus inside the daemon (a simple C++ publish/subscribe registry: a map from event name to a list of callbacks).
- [ ] **2.2** Emit the core events through it: `pane_created`, `pane_closed`, `pane_output`, `session_changed`, `window_changed`, `client_attached`, `client_detached`.
- [ ] **2.3** Expose `mux.on(event_name, callback)` to Lua via sol2, storing each Lua callback (a `sol::function`) in the event bus and invoking it from C++ when the event fires.
- [ ] **2.4** Define real `Pane` / `Window` / `Session` C++ classes exposed as sol2 `usertype`s (with properties like `id`, `cwd`, `title`, `pid`, `rows`, `cols`, `focused`), so Lua callbacks receive real typed objects instead of raw strings or IDs.
- [ ] **2.5** Design the plugin manifest format (e.g. `plugin.toml`: name, version, required API version, and a `[permissions]` table) and parse it with **toml++**.
- [ ] **2.6** Build the plugin loader: read the manifest, check the API version is compatible, and construct a restricted `sol::environment` for the plugin.
- [ ] **2.7** Implement permission enforcement — make sure a plugin manifest that doesn't request `shell = true` genuinely has no way to run shell commands from inside its sandboxed environment (don't register the shell-related functions into that plugin's environment at all).
- [ ] **2.8** Implement a simple plugin install mechanism v1: point at a git URL, clone it into `~/.config/mlux/plugins/` (via `fork()`/`exec("git", "clone", ...)`), auto-load on startup.
- [ ] **2.9** Build first-party plugin #1 (something simple and useful, e.g. a git-branch status widget) purely using the public plugin API, to sanity-check the API is actually usable.
- [ ] **2.10** Build first-party plugin #2 (e.g. a fuzzy session/window switcher) to stress-test the API from a different angle.
- [ ] **2.11** Build the structured automation socket: a separate `AF_UNIX` socket speaking JSON via **nlohmann::json**, distinct from the main daemon↔client protocol.
- [ ] **2.12** Implement a first batch of RPC methods on it: `pane.list`, `pane.info`, `session.list`.
- [ ] **2.13** Write the plugin API docs *as you build this phase*, not after — you'll catch API awkwardness much faster this way.
- [ ] **2.14** Get someone else (a friend, another dev) to write a small plugin using only your docs, and fix whatever confused them.

**Done when:** someone outside the project could write a nontrivial plugin using only your documentation, without reading your source code.

---

## Phase 3 — Session persistence, the cheap way first

**Goal: reboot survival that works today, with zero CRIU involved.**

- [ ] **3.1** Design the saved-session manifest format (JSON via nlohmann::json, or a small SQLite schema): layout tree, each pane's working directory, the command line that was running, environment variables, and pane titles.
- [ ] **3.2** Implement a "save session" command that serializes the daemon's current full state to disk in that format.
- [ ] **3.3** Implement a "restore session" path: when the daemon starts up with no existing sessions, check for a saved manifest and recreate the layout from it.
- [ ] **3.4** Implement restore-by-relaunch: for each saved pane, `chdir()` into its saved working directory and re-run its saved command line.
- [ ] **3.5** Handle the obvious failure cases gracefully: saved directory no longer exists, saved command no longer exists — show a clear message in that pane instead of silently failing.
- [ ] **3.6** Hook a clean shutdown/logout event (e.g. a `systemd` unit with `ExecStop=`, or a `logind` sleep/shutdown signal) to trigger an automatic save.
- [ ] **3.7** Implement a periodic background auto-save (a simple timer thread, or a `timerfd` on the daemon's event loop) as a safety net against unexpected crashes/power loss.
- [ ] **3.8** Full test: open several panes running different kinds of programs, reboot the machine for real, and confirm the layout comes back and every command relaunches correctly.

**Done when:** "my layout and sessions survive a reboot" reliably works — this is already a real, shippable feature before any CRIU code exists.

---

## Phase 4 — CRIU integration (the hard part)

**Goal: real running process state — not just the command line — survives a reboot, wherever that's genuinely possible.**

- [ ] **4.1** Revisit your Phase −1 spike notes; if you skipped Phase −1, do checkpoints −1.1 through −1.4 now, before writing any integration code.
- [ ] **4.2** Install CRIU's development headers (`libcriu-dev` or equivalent) and read through `criu.h` to learn its C API — functions like `criu_init_opts()`, `criu_set_pid()`, `criu_set_images_dir_fd()`, `criu_set_shell_job()`, `criu_dump()`, `criu_restore_child()`.
- [ ] **4.3** Write a small standalone C++ program (its own tiny Makefile target, separate from the main project) that links `-lcriu` and dumps/restores a plain process purely through that API — get this fully working in isolation first.
- [ ] **4.4** Build the **privileged helper** binary: a small, separate executable whose only job is to receive dump/restore requests (over its own `AF_UNIX` socket) and make the corresponding `libcriu` calls.
- [ ] **4.5** Define the request/response protocol between the main daemon and this helper (reuse the nlohmann::json approach from Section 2.11 for simplicity — e.g. `{"action": "dump", "pid": ..., "image_dir": ...}`).
- [ ] **4.6** Set up the actual privilege boundary: run the helper with the capabilities it needs (root, or `CAP_SYS_ADMIN`/`CAP_CHECKPOINT_RESTORE` set via `setcap` on the binary), while the main daemon stays an ordinary unprivileged user process.
- [ ] **4.7** Implement the pane risk-tier classifier: inspect a pane's process tree via `/proc/<pid>/fd` (readdir + readlink) and `/proc/<pid>/net/tcp*` and tag it `WARM`, `RESTART`, or `UNSUPPORTED`.
- [ ] **4.8** Wire the "save" flow: for `WARM`/`RESTART`-tier panes, send a dump request to the helper; for `UNSUPPORTED` panes, just record their metadata using Phase 3's manifest format.
- [ ] **4.9** Wire the daemon startup/restore flow: recreate PTYs first (`forkpty()` again, fresh), then for each saved pane check its tier and either (a) ask the helper to CRIU-restore it onto the new PTY, or (b) fall back to Phase 3's relaunch logic.
- [ ] **4.10** Implement the `PaneID` type (a small `struct PaneId { uint64_t value; };`, not a raw fd or path) so that nothing else in the codebase — config, plugins, UI — ever references a raw PTY device path directly; only the daemon's internal pane table knows the current device.
- [ ] **4.11** Build the transparency report: after a restore, show the user a simple per-pane list of `✓ WARM` / `⚠ RESTART` / `✗ UNSUPPORTED`.
- [ ] **4.12** Handle failure paths explicitly: a dump that fails (disk full, permission denied) should fall back cleanly to Phase 3-style metadata recording; a restore that fails should fall back to relaunching, with the reason logged somewhere the user can find it.
- [ ] **4.13** Run a full real-world test: a real working session (an editor with unsaved changes, a REPL with variables set, a build running), trigger a clean shutdown through your Phase 3.6 hook, physically reboot the machine, and confirm `WARM` panes come back with in-memory state intact.
- [ ] **4.14** Deliberately test the ugly edge cases: upgrade a system library between checkpoint and restore; try restoring when the original PID is already taken by something else; try checkpointing a pane with an open network connection and confirm it's correctly classified as `UNSUPPORTED` rather than silently failing.

**Done when:** you can shut down your own laptop mid-session and get back an editor with its unsaved buffer genuinely intact, a build still running where it left off, and clear, honest reporting on anything that had to restart instead.

---

## Phase 5 — Hardening, ecosystem, and polish

**Goal: something you'd trust with real work, and something other people could adopt.**

- [ ] **5.1** Build automated "golden output" tests using **Catch2** for the VT parsing layer: feed known escape sequences into libvterm, assert the resulting `Grid` state matches exactly.
- [ ] **5.2** Fuzz the input path around libvterm with random/malformed byte sequences (a simple harness is enough to start; `libFuzzer`/`AFL++` if you want to go further) and fix any crashes it turns up.
- [ ] **5.3** Build a small test matrix across real outer terminals (e.g. xterm, Alacritty, Kitty, GNOME Terminal) to catch `$TERM`/capability-negotiation bugs.
- [ ] **5.4** Stress-test the checkpoint/restore path directly: kill the privileged helper mid-dump, simulate a full disk during a dump, feed it a corrupted image directory, and confirm mlux degrades gracefully every time instead of hanging or corrupting other panes.
- [ ] **5.5** Run a security pass on the plugin sandbox: deliberately try to make a permission-less plugin touch the filesystem or network, and close any gaps you find.
- [ ] **5.6** Run a security pass on the privileged helper specifically: audit every input it accepts, and confirm it genuinely can't be made to do anything beyond checkpoint/restore.
- [ ] **5.7** Run the full test suite under AddressSanitizer, UndefinedBehaviorSanitizer, and (if you've added threading) ThreadSanitizer, and fix whatever they surface — do this as a routine `make sanitize` target, not a one-time pass.
- [ ] **5.8** Run `clang-tidy`/`cppcheck` over the whole codebase and clear the warnings that actually matter (use-after-free, leaks, obvious UB).
- [ ] **5.9** Implement the `mlux panic` command — an unconditional "kill everything and reset to clean" escape hatch that can't itself get stuck.
- [ ] **5.10** Write the full documentation site: installation, config reference, plugin API reference, and a clear explanation of the CRIU tier system for end users.
- [ ] **5.11** Build a small curated plugin index (a single file listing known-good plugins) and a short contribution guide.
- [ ] **5.12** Package it properly: add a `make install` target (copying binaries to `/usr/local/bin`, config templates to `/etc/mlux` or similar), then wrap that into a `.deb` via `dpkg-deb`, and/or write an AUR `PKGBUILD` that just calls `make`.
- [ ] **5.13** Write a migration guide for tmux users, plus ship the `require("compat.tmux")` keybinding-compat layer mentioned in the design doc.
- [ ] **5.14** Soft-launch to a small group of outside users, collect friction reports, and iterate.

**Done when:** someone other than you could install mlux, configure it, write a plugin for it, and trust it with a real working session.
