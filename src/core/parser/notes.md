# VT PARSER Notes

- The vt parser is supposed to be a `finite state machine` that handles all the different kinds of escape sequences


CSI format:
```
ESC [ <Private Marker> <Parameters> <Intermediate Bytes> <Final Character>
```

```
```

**States the parser can be in**:
- **Ground state** (the escape sequence hasn't started)
- Even when `ESC` is pressed, it is still in the ground state
- **CSI Entry**: When the `[` is parsed after the `ESC`, the state changes to `CSI Entry`
- **CSI Marker** : IF and after it finds a private Marker
- **CSI Param** : The integer entries separated by commas
- **CSI Intermediate** : The Intermediate bytes (if any)
- **CSI Final** : The final character that signifies what the CSI is actually doing

### Escape sequences:
- Those that start with `ESC` (`\x1b`)
- If the esc is followed by a `[`, it means it is a CSI

