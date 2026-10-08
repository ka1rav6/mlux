# Architecture


The `mlux` project is implemented in modern, object oriented c++. It uses `make` as the build system and `lua` as the plugin system
To understand in a detailed view what happens in mlux since the start, check (internal_flow.md)[internal_flow.md]

## Folder design

`src` contains all the source code of the main multiplexer.
`bench` contains all benchmarks compared to other multiplexers
`docs` contains all the important documentation of the project. Everything from the roadmap and this design, to the actual plugin interface documentation
`tests` contains all the unit test/ integration tests and other tests done to rigorously test each and every part of this project

All the different folders (except `docs` of course) have their dedicated `Makefile` that gets recursively called 

## Class Architecture and Design Decision

In very simple words, the `server` is actually a daemon handles multiple `sessions`. Each session is what the user detaches or attaches to.
Each session contains multiple `windows` for the user, and each window contains multiple `panes`. The panes are the *"split terminals"* that the user is actually seeing. 

The panes are drawn by `mlux` on the terminal. Every time the user does something, the bytes that are received by the terminal is parsed by the `TerminalParser`. These could be of the `csi` type, `osc` type, or any other type (including just normal bytes).

The window contains all the `geometry` and layout of the panes. It, hence, also knows which pane is the active one and where the data was written/ should be written back to.

The geometry is just the data of points and sizes required to define a pane
`Layouts` of the panes are stored through a tree. Suppose if my current structure is something like this:

```
|-----------------|---------------|
|                 |               |
|                 |               |
|                 |               |
|-----------------|               |
|                 |               |
|                 |               |
|                 |               |
|-----------------|---------------|
```

Then, the top most pane layout node, has two children nodes (of a vertical split) for the left and the right panes. The left pane has two more children that are separated because of a horizontal split



WHY I chose to have a base parser:
I want there to be one main parser that just hands the current bytes to the particular parser (csi/osc etc) until the state changes.
So, whenever there is a user keypress:
  - client reads its own stdin
  - IPC to daemon
  - daemon: which session/window/pane is active?       (Session::activeWindow, Window::activePane)
  - encode the key for that pane's current modes       (TerminalModes::applicationCursorKeys)
  - Pane::sendInput(bytes)
  - Pty::write()  →  PTY master  →  child's stdin

The child writes to PTY slave
  - epoll says "master fd N is readable"
  - look up which Pane owns fd N
  - Pty::read()
  - Terminal::feed(bytes)
  - Parser::feed() → processByte() per byte, dispatching by state
  - mutates that pane's Screen / Cursor / TerminalModes accordingly
  - renderer diffs Screen against what the client last saw (only the differences)
  - IPC to client
  - client writes to the user's real terminal


So the parser does nothing when there is a user keypress. It only works on the way back.
