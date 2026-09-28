# C8-ass

Custom assembler for my [CHIP-8 emulator](https://github.com/tackx/c8) (WIP)

Developed live at:

- <https://www.twitch.tv/tack___>
- <https://www.youtube.com/@Txck-dev>

![code](/docs/vxn.jpg)

## TODO

- [ ] Fix relative paths in tests
  - [ ] Lexer should parse quoted text as one token (no matter its contents)
- [ ] Add colored output
  - [ ] Only use it if stdout is not a file
- [ ] Fix up/refactor the Timer class
- [ ] Make paths for the `INCBIN` directive relative to the source file?
- [ ] Max ROM size limit?
- [ ] Add "squiggles" + caret to error messages?
- [ ] Test setting this up on Linux
- [ ] Update README
  - [ ] Add some more screenshots of the CLI output
- [ ] Add support for expressions?
- [ ] Full support for chipper syntax?
