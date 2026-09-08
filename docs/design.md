# mlux — Design Document

*(mlux = "mux" + "Lua". Working name for the project.)*
*(Stack: C++ with plain Makefiles — no Rust, no Cargo.)*

This document explains **what mlux is, how it's built, and why it's built that way.** It's written in plain language on purpose — it should make sense whether you wrote the code or you're just skimming it before a design review.

---

## 1. What is mlux, in one paragraph

mlux is a terminal multiplexer — a tool that lets you split your terminal into many panes, organize them into windows and sessions, and keep everything running in the background even if you close your terminal. It's the same category of tool as tmux. What makes mlux different is: (1) it's configured and scripted in real Lua instead of a custom command language, (2) it has a proper plugin system with real permissions instead of "plugins are just shell scripts," and (3) it can save a session's actual running state — not just "what command was running" — using a Linux feature called CRIU, so your work can survive a full shutdown and reboot, not just a dropped connection.

---

## 2. Core architecture

### 2.1 One server, many clients

mlux runs as a **daemon** (a background server process) that owns everything: your sessions, windows, panes, and the actual programs running inside them. Your terminal window just runs a small **client** that connects to this daemon over a Unix domain socket, sends it your keystrokes, and draws whatever the daemon tells it to draw.

This is the same trick tmux uses, and it's *why* multiplexers survive disconnects at all: if your SSH connection drops, only the client dies. The daemon — and everything running inside it — keeps going untouched.

```
[your terminal] <---> [mlux client] <---socket---> [mlux daemon] <---> [panes: shells, editors, etc.]
```

### 2.2 What a "pane" actually is

Each pane is:
- A **PTY** (a virtual terminal device), created with the POSIX `openpty()`/`forkpty()` calls — one side is held by the daemon, the other side is given to a program (usually your shell) as its "terminal."
- A small **terminal emulator built into the daemon itself**, which reads the raw bytes coming from that program (color codes, cursor movement, etc.) and turns them into a grid of characters — this is the actual "screen" that gets displayed. mlux is emulating a terminal on the inside, just like tmux does.

### 2.3 Sessions, windows, and panes

```
Server
 └── Session       – a named workspace
      └── Window   – one tab; can be split into panes
           └── Pane – one running program + its own PTY
```

Panes inside a window are arranged in a tree (every split is either "divide left/right" or "divide top/bottom"), which is what lets you resize things proportionally and save/restore layouts easily. In the C++ code, this is a straightforward recursive class (`SplitNode` containing either a `Pane` or two child `SplitNode`s).

### 2.4 Everything goes through one typed command path

In tmux, *everything* — key bindings, plugin actions, hooks — eventually turns into a text command string like `split-window -h`. That's flexible, but it also means every part of the system is passing strings around and re-parsing them.

**mlux does not do this.** Internally there is a typed C++ API, and Lua is a thin, ergonomic layer on top of it:

```lua
mux.options.status.enabled = true
mux.map("C-h", mux.focus.left)

mux.on("pane_created", function(pane)
  print("New pane: " .. pane.id)
end)
```

There is no intermediate "turn this into a command string, then parse it back" step. Plugins and key bindings call the same real C++ objects your core code uses, exposed into Lua directly (see Section 12 for how).

---

## 3. Configuration and scripting

### 3.1 Lua instead of a custom config language

tmux's config language (`.tmux.conf`) is deliberately simple, which makes small configs easy but makes anything complex ugly — you end up generating command strings in loops just to do repetitive setup.

mlux config is just Lua, so you get real control flow for free:

```lua
for i = 1, 5 do
  mux.map("M-" .. i, function()
    mux.select_pane(i)
  end)
end
```

### 3.2 Config reloads instantly

Editing `~/.config/mlux/init.lua` and reloading should never require restarting the daemon or losing any panes. This matters a lot in practice — config-as-code only feels good if changing it is instant.

### 3.3 Project-specific config

A developer working across many projects often wants different layouts, bindings, or plugins per project. mlux supports this directly: when you enter a directory, it looks for a `.mlux.lua` file there, loads it, and merges it on top of your global config.

```
~/projects/api-server/.mlux.lua   → this project's own layout & plugins
~/projects/mobile-app/.mlux.lua   → a totally different setup
```

This is something tmux can't really do natively, and it matches how people actually work — one global config doesn't fit every project.

---

## 4. The event system (replacing tmux's "hooks")

tmux has "hooks" — but a hook just runs another tmux command. There's no real data passed around, just text.

mlux gives plugins and config code **real objects**, not text:

```
event fires
   ↓
typed callback function runs
   ↓
receives a real Pane / Window / Session object
```

This makes automation dramatically cleaner. For example, reacting to output in a pane:

```lua
mux.on("pane_output", function(pane, data)
  if data:match("BUILD FAILED") then
    mux.notify("Build failed in " .. pane.title)
  end
end)
```

### 4.1 Events instead of polling

Because tmux plugins don't have a strong event system, many of them just poll on a timer — "every second, run a shell command, check something, update the status bar." That's wasteful (it forks a process a lot) and always a little bit stale.

mlux plugins **subscribe to events** instead:

```
git branch changes  →  event fires  →  status widget updates
```

instead of:

```
every 1 second: check git branch anyway, even if nothing changed
```

Internally, this event bus is just a simple C++ publish/subscribe registry (a map from event name to a list of callbacks), with Lua callbacks stored as opaque references into the Lua state and invoked from C++ when an event fires.

---

## 5. The status bar is made of widgets, not format strings

tmux's status bar works by evaluating format strings like `#{session_name}` and `#{pane_current_path}` on a timer. That's fine for simple things, but building a complex, dynamic status bar this way gets awkward fast.

In mlux, status bar segments are small Lua programs:

```lua
mux.status.add({
  render = function(ctx)
    return { text = ctx.session.name, fg = "#ffffff" }
  end,
  update_on = { "session_changed", "pane_changed" }
})
```

Each widget declares exactly which events it cares about, so it only re-renders when something relevant actually changes.

---

## 6. Plugin system

### 6.1 The problem with tmux plugins

tmux "plugins" (via TPM) are really just git repos full of shell scripts. The flow looks like this:

```
plugin → shell script → tmux CLI → tmux parses the string → state changes
```

Problems this causes:
- Constant process spawning (slow)
- Fragile text-parsing between plugin and multiplexer
- No versioning contract — plugins silently break across tmux versions
- No sandboxing at all — a "plugin" can run literally any shell command, including deleting your files

### 6.2 mlux's plugin model

```
Lua plugin → typed API → directly reads/changes mlux state
```

Plugins target a specific **API version**, and mlux guarantees compatibility within that version:

```lua
mux.api_version -- e.g. 1
```

### 6.3 Plugins declare permissions up front

Every plugin ships a manifest describing exactly what it's allowed to touch:

```toml
name = "git-status"
version = "1.2.0"
api = "1"

[permissions]
pane_read = true
filesystem = false
network = false
shell = false
```

The plugin's Lua environment is built to match — if `shell = false`, the plugin's sandbox simply doesn't have access to run shell commands, at the language level, not just "please don't." In C++ this is done by constructing a **separate, restricted Lua global table per plugin** (rather than letting every plugin share the same globals as the core config), populated only with the functions its manifest actually permits.

### 6.4 A simple plugin registry, not a walled garden

v1 doesn't need an app-store-style plugin marketplace. Point mlux at a git URL, like tmux does — but now the manifest and API version mean plugins can actually be checked for compatibility instead of just "hope it still works." A small, curated, PR-able index (a single file listing known-good plugins) is a cheap and valuable v2 addition.

---

## 7. Terminal compatibility (the part almost nobody gets fully right)

### 7.1 Why this is hard at all

A multiplexer sits *between* two terminal emulators:

```
Application → mlux's internal terminal emulator → mlux → the real terminal emulator you're looking at
```

Every layer in that chain can misunderstand a feature — truecolor, Kitty's image protocol, Sixel graphics, OSC 8 hyperlinks, synchronized output, focus events. Historically, this is exactly where tmux has struggled, because it has to explicitly learn about each new terminal feature to pass it through correctly.

### 7.2 mlux's approach: capabilities as real data, not string matching

Never do this:

```cpp
if (terminal_name == "kitty") { ... }
```

Do this instead — check an actual capability flag:

```cpp
if (capabilities.kitty_graphics) { ... }
```

`TerminalCapabilities` is a first-class struct in mlux, tracked separately for:
- What the **outer** terminal (the one you're actually looking at) can do
- What mlux **presents** to the program running inside a pane

Keeping these two explicitly separate prevents the classic bug where a program thinks it can use a feature that the real terminal can't actually render, or vice versa.

### 7.3 `$TERM` handling

tmux presents itself to programs as `TERM=tmux-256color`, while the terminal you're actually looking at reports something else (`xterm-256color`, `alacritty`, etc.). If capability negotiation between these two isn't handled carefully, features silently disappear or misbehave. mlux treats this negotiation as its own tested subsystem, not scattered `if` statements.

### 7.4 A terminal compatibility test lab

Build automated tests in both directions:

```
known application output → mlux → expected internal terminal state
```

```
a given outer-terminal capability → mlux → what it correctly presents inward
```

This is how serious terminal projects (Alacritty, WezTerm) catch regressions before users do, and mlux should do the same from the start rather than bolting on tests later.

---

## 8. Automation and AI-agent friendly API

### 8.1 The problem with a text-only CLI

tmux's CLI (`send-keys`, `capture-pane`) is great for humans and shell scripts, but any external tool — especially an AI coding agent — has to deal with raw text output. That means screen-scraping ANSI codes just to find out what's in a pane.

### 8.2 mlux's solution: a structured RPC socket

Alongside the normal CLI, mlux exposes a separate Unix socket that speaks structured JSON requests and responses:

```json
// request
{ "method": "pane.list" }

// response
{
  "panes": [
    { "id": 4, "cwd": "/home/user/project", "pid": 18342, "rows": 40, "cols": 120, "focused": true }
  ]
}
```

An external tool or agent gets **real, structured state** — no `capture-pane` plus a hand-written ANSI parser required. This is one of mlux's strongest differentiators, since more and more tools (including AI coding agents) are already driving tmux this way today, just clumsily.

---

## 9. Session persistence and CRIU integration

This is mlux's headline feature: sessions that survive not just a dropped connection, but a full shutdown and reboot — with the actual running program state preserved, not just "what command was running."

### 9.1 Why tools like tmux-resurrect aren't enough

`tmux-resurrect` and similar tools only remember: the command that was run, the working directory, the layout, and environment variables. On "restore," they just **re-run the command from scratch**. They do not preserve:

- Heap and stack memory
- CPU register state
- Open file descriptors
- Any in-memory application state (an editor's undo history, a REPL's variables, a long computation's progress)

That gap — restarting a command vs. actually resuming it — is exactly what mlux closes using **CRIU** (Checkpoint/Restore In Userspace), a Linux tool that can freeze a running process tree and later resume it as if it never stopped.

### 9.2 The hardest problem: PTYs don't survive a reboot

After a reboot, the old PTY device (say `/dev/pts/4`) is simply gone. A freshly restored process needs to be reattached to a **brand new** PTY. This single problem — reconnecting a CRIU-restored process to a new terminal device — is the hardest and most important piece of engineering in the whole project. It is a known, documented pain point in CRIU itself, not a hypothetical concern.

**mlux's answer: make PTY identity an abstraction the rest of the system never touches directly.**

```
Pane 7
  before reboot: backed by /dev/pts/4
  ... reboot ...
  after restore: backed by /dev/pts/2
```

Nothing in mlux — config, plugins, the UI — ever refers to a raw device path. Everything refers to a stable `PaneID` (in C++, a small opaque struct wrapping an integer, not a raw fd or path). The daemon is the only thing that knows which physical PTY currently backs which pane, and it's free to swap that out silently during a restore.

### 9.3 Not every process can be resumed — and that's fine

CRIU has real limits, and mlux is upfront about them instead of pretending they don't exist:

- **Same PID required on restore** — if something else already holds that PID, restore fails.
- **PTY reattachment is fragile** — the exact problem in 9.2.
- **IPC (shared memory, semaphores) needs its own namespace** to be checkpointed safely.
- **Library/version drift** — if the system's shared libraries change between checkpoint and restore (e.g. an `apt upgrade` happened), a resume can fail or misbehave.
- **Open network connections basically never survive a reboot** (new IP, dropped Wi-Fi, the remote side may have given up).
- **No GPU support at all.**
- **Root or special permissions are generally required.**
- **Filesystem paths must still exist** at the same locations on restore.

Rather than fighting all of this, mlux classifies every pane into one of three tiers before attempting anything:

| Tier | Meaning | What mlux does |
|---|---|---|
| **✓ WARM** (fully resumable) | Plain shells, editors, REPLs, local builds — no network/GPU dependency | CRIU-checkpoint and restore it for real |
| **⚠ RESTART** (best-effort) | Has local file handles but no risky dependencies; CRIU attempted, may fall back | Try CRIU first; if it fails, restart the command cleanly instead |
| **✗ UNSUPPORTED** (restart-only) | Open network sockets, GPU contexts, GUI/display connections, denylisted commands (ssh, live DB sessions) | Never attempt CRIU — just remember the command, cwd, and env, and re-launch it fresh |

mlux checks a pane's open file descriptors (`/proc/<pid>/fd`) and network connections (`/proc/<pid>/net/tcp*`) before deciding which tier it falls into, so most panes get classified automatically without asking the user anything.

### 9.4 Being honest with the user is itself a feature

mlux never promises "everything will come back exactly as it was." It promises: **"we preserve process state whenever the process is actually checkpoint-compatible, and we tell you clearly when it isn't."** After a restore, the user sees something like:

```
Pane 1   ✓ WARM       (vim, unsaved buffer intact)
Pane 2   ✓ WARM       (python REPL, variables intact)
Pane 3   ⚠ RESTART    (build script, re-launched from scratch)
Pane 4   ✗ UNSUPPORTED (ssh session, re-launched)
```

This transparency builds trust in a way that silently failing (or silently pretending everything worked) never could.

### 9.5 When checkpoints happen

- **Manual save** command, any time.
- **On clean shutdown/logout**, via a system hook — this is the main "survive a laptop shutdown" case, and the most reliable trigger since the machine is still fully alive when it runs.
- **Periodic background saves**, so an unexpected power loss doesn't lose everything (accepting that a periodic snapshot is always a little stale).
- **On suspend**, as a nice-to-have, hooked separately from shutdown.

### 9.6 Never checkpoint the daemon itself

It's tempting to think "why not just checkpoint the whole mlux process?" Don't. The daemon holds sockets, possibly-stale file descriptors, and a live Lua VM with plugin state — all much messier to make checkpoint-safe than a plain shell's process tree. Instead: **restart the daemon fresh on boot, recreate PTYs, and CRIU-restore only the individual pane process trees onto those new PTYs.**

### 9.7 Privilege and security

CRIU generally needs elevated permissions. mlux keeps this contained: the main daemon runs as a normal user process, and only a small, separately auditable **privileged helper** binary is allowed to actually invoke CRIU (via `libcriu`, CRIU's official C client library — see Section 12). This keeps the much larger, plugin-hosting daemon out of the business of running as root. Checkpoint image files also contain full memory dumps of your programs — meaning potentially secrets or sensitive data — so they're stored with strict file permissions, and encryption at rest is worth considering if they're kept long-term.

---

## 10. Modern, sane defaults

tmux's default keybinding (`Ctrl-b` as the prefix) comes from its GNU Screen ancestry and isn't intuitive to most newcomers. mlux ships with a modern default keymap (e.g. using `Alt`/`Super` combinations for split/navigate/new-window/close), while still offering a compatibility layer:

```lua
require("compat.tmux")
```

for anyone migrating over who wants their tmux muscle memory to keep working.

---

## 11. Error recovery: the "get me out" button

A multiplexer is infrastructure — if it ever gets into a weird state, the user needs an escape, not a debugging session. mlux ships a guaranteed-to-work panic command:

```
mlux panic
```

which kills everything and returns you to a clean slate, no matter what odd intermediate state a bad restore or plugin bug left things in. This matters more for user trust than squeezing out one more percent of restore success rate — however good the CRIU logic gets, edge cases will happen, and there always needs to be a way out that can't itself get stuck.

---

## 12. Tech stack (C++ / Makefiles)

| Piece | Choice | Why |
|---|---|---|
| Core daemon | **C++ (C++20)** | Matches your stack. Modern C++ (RAII, smart pointers, `std::optional`/`std::variant`) gets you most of the way toward safe resource handling for PTYs, sockets, and Lua state, even without Rust's compile-time guarantees. |
| Build system | **Plain Makefiles** — one per component (`daemon/`, `client/`, `common/`, `helper/`) plus a root Makefile that recurses, using `pkg-config` to locate system libraries | Matches your stated preference, keeps the whole build simple and inspectable, no external build-tool dependency beyond `make` and `pkg-config`. |
| Lua embedding | **Lua 5.4 (liblua)** + **sol2** (a header-only C++ binding library on top of the Lua C API) | sol2 gives you ergonomic, type-safe bindings (`mux.map`, `mux.on`, custom userdata types for `Pane`/`Window`/`Session`) without needing anything heavier than adding a header and linking `-llua5.4`. |
| Terminal parsing | **libvterm** | A proven, widely used C library for VT100/ANSI parsing (used by Neovim, Kitty's underlying tooling, WeeChat). Don't hand-roll ANSI parsing — reuse a battle-tested implementation; this is the single most bug-prone part of any terminal project if built from scratch. |
| PTY handling | **POSIX `openpty()`/`forkpty()`** (from `<pty.h>`) wrapped in a small RAII class that closes fds and reaps the child on destruction | Native, dependency-free, and exactly what tmux itself is built on under the hood. |
| Checkpoint/restore | **libcriu** (CRIU's official C client library), called from a small privileged helper binary | libcriu speaks CRIU's RPC protocol for you (you call functions like `criu_dump()`/`criu_restore()` and set options like `criu_set_shell_job()`), so you don't need to hand-roll the underlying protobuf messages yourself. |
| Daemon↔client protocol | A small **custom length-prefixed binary protocol** over Unix domain sockets (plain structs, manually serialized) | Keeps the hot path — screen redraw updates — fast and dependency-free; this is internal and doesn't need to be "nice," just fast and correct. |
| External automation API | **nlohmann::json** (header-only) over a separate Unix socket, JSON-RPC-style | Header-only, trivial to drop into a Makefile build, and ergonomic for anyone outside the project (including AI agents) scripting against mlux. |
| Plugin manifests | **toml++** (header-only TOML parser) | Keeps plugin manifest files human-friendly and easy to hand-edit, with no extra runtime dependency. |
| Session metadata | **SQLite** (`-lsqlite3`, either the plain C API or a very thin RAII wrapper you write yourself) for queryable history, plus JSON exports (via nlohmann::json) for human-readable restore transparency | Mature, embedded, zero extra server process, and easy to link from a Makefile. |
| Plugin sandboxing | Per-plugin restricted Lua environments via `sol::environment`, populated only with the functions a plugin's manifest permissions allow | Same underlying idea regardless of host language — Lua's design makes this practical to enforce properly. |
| Testing | **Catch2** (header-only test framework) for unit/golden-output tests | Drops straight into a Makefile-based project with no extra build tooling. |
| WASM (later, optional) | **wasm3** (a small, embeddable WASM interpreter written in C) if/when you want non-Lua plugins | Lightweight enough to add to a Makefile build without pulling in a heavyweight toolchain; a v2+ concern, not a v1 requirement. |

### 12.1 Engineering discipline this stack needs that Rust would have given you for free

Since C++ doesn't catch memory-safety mistakes at compile time the way Rust does, budget real, ongoing engineering time for the habits that compensate for that — this isn't optional polish, it's load-bearing for a long-lived daemon that manages raw file descriptors and hosts third-party plugin code:

- **RAII everywhere.** Every PTY fd, every socket, every `lua_State*`, every CRIU image directory handle should be owned by a class whose destructor cleans it up — never a bare `int fd` or raw pointer passed around and closed "by hand" in multiple places.
- **Smart pointers, not raw owning pointers.** `std::unique_ptr` for single ownership (most of your `Pane`/`Window`/`Session` tree), `std::shared_ptr` only where something is genuinely shared (e.g. a `Window` linked into multiple sessions, mirroring tmux's `winlink` design in Section 2.3).
- **Sanitizer builds as a routine part of development, not a pre-release checkbox.** Build and run your daily test suite with AddressSanitizer and UndefinedBehaviorSanitizer on regularly (a `make sanitize` target is worth adding on day one), and add ThreadSanitizer once you introduce any real concurrency (e.g. the event bus or the privileged helper's IPC handling).
- **Static analysis in the loop.** Run `clang-tidy` and/or `cppcheck` as part of your normal workflow, not just occasionally — catching a use-after-free or a missing bounds check before it ships matters even more here than in most C++ projects, given this daemon runs continuously and touches sensitive state (checkpoint images, plugin code).
- **Fuzz the terminal parser and the daemon↔client protocol parser specifically** (Section 7.4 and Phase 5 of the roadmap) — these two places parse untrusted-shaped input (arbitrary program output, arbitrary client messages) continuously, and are exactly where a memory-safety bug would matter most.

**Platform note:** CRIU is Linux-only, independent of language choice — mlux should be Linux-first (or Linux-only for v1), with the rest of the multiplexer designed so it *could* run elsewhere later, minus the checkpoint feature.

---

## 13. Why this ends up better than tmux (the short version)

| tmux's limitation | mlux's answer |
|---|---|
| Everything is a text command string | Typed C++ API, Lua is a thin layer over it |
| Plugins are shell scripts poking at the CLI | Real typed plugin API with a version contract |
| No plugin sandboxing at all | Manifest-declared permissions, enforced via per-plugin restricted Lua environments |
| Modern terminal features are painful to support | Explicit `TerminalCapabilities` struct, tested both directions |
| Config language is deliberately limited | Full Lua, real loops/functions/conditionals |
| Hooks just run more commands | Real typed events with real objects |
| Plugins often poll on a timer | Event-driven updates only |
| Status bar is a format-string mini-language | Status bar widgets are real Lua functions |
| No per-project configuration | `.mlux.lua` auto-loaded per directory |
| CLI is textual, awkward for automation/agents | Structured JSON-RPC socket alongside the CLI |
| "Resurrect" tools only restart commands | CRIU-based real process state restoration |
| PTY identity is assumed stable | PTY identity is abstracted behind a stable `PaneID` |
| No guaranteed escape hatch | `mlux panic` — a guaranteed clean reset |
