# Multiplayer Tic-Tac-Toe Game

This repository contains the source code and related files for Multiplayer Tic-Tac-Toe Game. The project involves a grid-based game where multiple players compete to mark positions on the grid. The game is implemented in C and uses inter-process communication (IPC) to handle player interactions.

## Project Structure

```
.
├── Ceng334___Homework_1.pdf
├── game_structs.h
├── input_linux.txt
├── input.txt
├── main.c
├── Makefile
├── output.txt
├── print_output.c
├── print_output.h
├── server
├── server.c
├── players_linux/
│   ├── diagonal_player
│   ├── horizontal_player
│   ├── random_player
│   ├── smart_player
│   ├── vertical_player
├── players_macos/
│   ├── diagonal_player
│   ├── horizontal_player
│   ├── random_player
│   ├── smart_player
│   ├── vertical_player
```

## Files

- **game_structs.h**: Defines the data structures used for client and server messages, grid positions, and grid updates.
- **input_linux.txt**: Input file specifying the game configuration and player details for Linux.
- **input.txt**: Input file specifying the game configuration and player details for macOS.
- **main.c**: Main program that initializes the game, sets up communication channels, and handles player interactions.
- **Makefile**: Build script to compile the project.
- **output.txt**: Sample output file showing the game progress and results.
- **print_output.c**: Contains the function to print client and server messages, and grid updates.
- **print_output.h**: Header file for `print_output.c`.
- **server**: Executable file for the server.
- **server.c**: Server implementation that handles client messages and updates the game state.
- **players_linux/**: Directory containing player executables for Linux.
- **players_macos/**: Directory containing player executables for macOS.

## How to Build

To build the project, run the following command:

```sh
make
```

This will compile the source files and generate the `server` executable.

## How to Run

To run the game, execute the `server` program:

or for Linux:

```sh
./server < input_linux.txt
```

or for MacOS:
```sh
./server < input_macos.txt
```
