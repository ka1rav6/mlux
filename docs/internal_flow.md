# The complete flow, step by step

**Step 1** : 
you type mlux. 
This starts a client process. It tries to `connect()` to /run/user/1000/mlux/default.sock. Nothing’s there, so the connect fails.

**Step 2** :
the client starts the daemon. 
It forks; the child calls `setsid()` to cut itself loose from your terminal, then execs `mlux --daemon`. 
The daemon creates the socket, listens, and enters its epoll loop. The client retries the connect and gets in.

**Step 3** : the client takes over your terminal. Saves your `termios`, switches to raw mode (so Ctrl-C becomes a byte instead of killing it), asks the kernel for the terminal size via `TIOCGWINSZ` (say 80×24) and enters the alternate screen.

**Step 4** : the client says hello. It sends one message: ATTACH, session="default", size=80x24.

**Step 5** : the daemon builds the session. No session called “default” exists, so:

create a Session, which creates a Window, which creates a Pane
Pty::spawn(ProcessSpec{"/bin/bash", ...}) -> forkpty() -> the child execs bash
the daemon now holds master fd 7 and child pid 4242
add fd 7 to epoll, and record 7 -> that Pane in its index
create Client{socket=5, attached=session1, viewport=80x24}
Layout::calculate(80x24) -> the one pane gets the whole area
Pty::resize(80x24) -> TIOCSWINSZ, so bash knows how wide it is

Note what the client did here: nothing. It asked, and the daemon did all of it.

**Step 6** : bash prints its prompt. bash writes user@box:~$ to its end of the PTY. On the daemon side, epoll_wait returns “fd 7 is readable.” The daemon looks up fd 7 -> Pane, calls Pty::read, then Terminal::feed(bytes). The parser walks the bytes and writes characters into Screen’s cells and moves the Cursor. The pane is marked dirty.

Nothing has been displayed yet. The prompt now exists only as cells in the daemon’s memory.

**Step 7** : the daemon renders. For each client attached to that session, it diffs Screen against client.last_rendered, sends only what changed over the socket, and updates last_rendered.

**Step 8** : the client draws. It receives the changed cells, turns them into escape sequences, writes them to its stdout. Now you see the prompt.

**Step 9** : you press l. The client reads byte 0x6c from its stdin and sends INPUT, 0x6c. The daemon asks: which pane? -> client.attached -> session -> activeWindow() -> activePane(). Then Pane::sendInput -> Pty::write -> fd 7 -> bash’s stdin. bash echoes the l back out, and we’re at step 6 again.

So every keystroke is a full round trip: client -> daemon -> bash -> daemon -> client.

**Step 10** : you detach. The daemon sends DETACH and drops the Client object. The client restores your termios, leaves the alternate screen, and exits.

The daemon, the session, and bash are all still running. bash didn’t notice a thing : it’s still sitting on its PTY, which nobody closed.

**Step 11** : you type mlux again, in a different terminal that’s 100×30. New client process, connects (daemon’s already up), sends ATTACH, "default", 100x30. The daemon finds the existing session and makes a new Client whose last_rendered is empty : so the diff is “everything,” and you get a full repaint. It runs Layout::calculate(100x30) and Pty::resize, bash gets a SIGWINCH and redraws.

You see your shell exactly as you left it : because the Screen was in the daemon the entire time.
