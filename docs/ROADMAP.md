# ROADMAP

## Part 1 : Build a working terminal multiplexer:
By the end of this part, we will have a fully working terminal multiplexer that can perfectly work for day-to-day things.

- [ ] all the basic classes 
- [ ] process/pty wrapper
- [ ] `mlux` daemon initializer 
- [ ] VT Parser -> all the different parsers within (csi, etc)
- [ ] Argument parser integration
- [ ] `mlux` command parsing
- [ ] Physical pane drawing
- [ ] Handling important escape sequences etc
- [ ] Having a commands dispatching key 
- [ ] Session detaching
- [ ] Session revisiting
- [ ] Ever running daemon
- [ ] Signal handling (redirecting)
- [ ] Window resizing should not break `mlux`


## Part 2 : Making it highly configurable using `lua`:

- [ ] Exposing everything to the `lua` API
- [ ] A good configuration system with a defined configuration folder.
- [ ] Easy plugin system with documentation
- [ ] Other configuration environment (such as `leader key` etc)
- [ ] Hot reload without session loss/ server reload

## Part 3 : `CRIU` Integration for `Linux` :

- [ ] Making classes serializable
- [ ] Simple protocol for saving session data
- [ ] Saving sessions using `CRIU` 
- [ ] Clearly categorizing panes (processes) that should be serialized and panes that should be restarted
- [ ] Restoring sessions post-restart

## Part 4: Modernization:
- [ ] support for kitty and other terminal emulators
- [ ] support for kitty graphics protocol
- [ ] support for `windows` and `mac`

