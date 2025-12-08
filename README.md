# netchat

A minimal terminal-based TCP chat program with separate client and server modes. The client renders a lightweight UI for composing messages, while the server multiplexes connections and broadcasts messages to all peers.

## Table of Contents
- [Features](#features)
- [Getting Started](#getting-started)
- [Running the Server](#running-the-server)
- [Running the Client](#running-the-client)
- [Project Layout](#project-layout)
- [Development](#development)
- [Screen Recording](#screen-recording)
- [License](#license)

## Features
- **Two executable modes**: start the binary as either a chat **server** or **client** using a single entry point. `main` parses subcommands and options to pick the right path at runtime.
- **TCP broadcast server**: accepts multiple connections, acknowledges each message, and broadcasts incoming text to every connected client using `poll(2)` for multiplexing.
- **Interactive terminal client**: switches the terminal to cbreak mode, handles signals (quit, interrupt, suspend), and renders colored message/input boxes with basic scrolling for longer sessions.
- **Configurable endpoints**: choose custom host/port pairs for both server and client; sensible defaults (`127.0.0.1:8080`) make local testing quick.
- **Lightweight build**: plain C11 code with the standard library and POSIX sockets—no external dependencies beyond a compiler and Make.

## Getting Started
### Prerequisites
- POSIX-like environment (Linux, macOS, or WSL)
- `gcc` or another C11-compatible compiler
- `make`

### Build
```bash
make
```
This compiles all `.c` sources into a single `main` binary in the repository root.

## Running the Server
Start the server on the default loopback address:
```bash
./main server
```
Specify a host/port explicitly (for remote clients, bind to `0.0.0.0`):
```bash
./main server -h 0.0.0.0 -p 9090
```
Key behaviors:
- Listens for new TCP connections and adds each client to a shared poll list.
- Prints server activity (connections, messages, disconnects) to the terminal.
- Sends an acknowledgment (`"Received your message\n"`) back to the sender before broadcasting the message to every other client.

## Running the Client
Connect to a server (defaults shown):
```bash
./main client -h 127.0.0.1 -p 8080
```
Client experience:
- Terminal switches to cbreak mode for responsive single-keystroke input.
- Use **Enter** to send the current line; messages are dispatched with `send(2)` and displayed in the message pane.
- Incoming server messages appear in a separate colored box, with basic scrolling when the terminal nears the bottom.
- Gracefully handles `Ctrl+C`, `Ctrl+\`, and `Ctrl+Z` by restoring terminal settings before exit or suspend.

## Project Layout
- `main.c` — Entry point, argument parsing, and mode selection.
- `server.c` / `server.h` — TCP server setup, polling, acknowledgments, and broadcasting.
- `client.c` / `client.h` — Terminal UI, input handling, signal management, and socket I/O for the chat client.
- Supporting utilities:
  - `box.c`, `modes.c`, `tutil.c` — Terminal rendering, cursor control, and input helpers.
  - `plist.c` — Dynamic pollfd list used by the server loop.
  - `log.c`, `errfunc.c`, `check.h` — Logging and error-handling helpers.
- `makefile` — Simple C11 build with `make`.

## Development
- Build: `make`
- Run server: `./main server [-h <host>] [-p <port>]`
- Run client: `./main client [-h <host>] [-p <port>]`
- Clean artifacts: `make clean`

## Screen Recording
Include a short screen recording that demonstrates a client connecting and exchanging messages with the server. Add your link or embed here:
> _[Upload or link your recording]_ 

## License
This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
