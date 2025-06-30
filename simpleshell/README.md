# SimpleShell

`SimpleShell` is a mini UNIX shell implementation in C with support for command execution, pipes (`|`), background processes (`&`), and history tracking.

## Features

- Run basic UNIX commands like `ls`, `cat`, `grep`, etc.
- Support for piped commands: `ls | grep txt`
- Support for background execution: `sleep 5 &`
- Built-in command: `cd`
- View command history with `history`
- Process termination logging in `termination.txt`
- Handles `Ctrl+C` to exit gracefully

## File Overview

- `SimpleShell.c`: Core shell implementation
- `fib.c`: Sample test program
- `simpleshell`: Compiled binary
- `history.txt`: Logs command history
- `termination.txt`: Logs terminated process info

## How to Compile & Run

```bash
gcc SimpleShell.c -o simpleshell
./simpleshell
```

## Example Commands

```bash
ls
wc -l SimpleShell.c
cat SimpleShell.c | grep printf
./fib 10
sleep 3 &
echo hello world
sort SimpleShell.c | uniq
history
exit

```

## Known Quirks

- Fast commands may show `Duration: 0.00 sec` in logs
- Very rarely, the prompt may duplicate (`SimpleShell$ SimpleShell$`) — purely cosmetic

## Notes

- Use `Ctrl+C` to exit — shell handles it cleanly and prints termination log
- All output is logged to `termination.txt` and `history.txt`
- Intended for OS course assignment (IIIT-Delhi)

---