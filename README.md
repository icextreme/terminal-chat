# Terminal Chat (t-chat)
Terminal Chat is a multithreaded chat application for Linux that allows for terminal-to-terminal communication between two users. This C program utilizes `pthreads` for concurrent communcation through UDP.

## Getting Started

### Prerequisites
A Linux machine with `gcc` and `pthreads` installed.

### Building
Using the terminal, navigate to `terminal-chat/src/` and type `make` to compile the program. 

To remove the executable file, as well as other object files, type `make clean`.

## Usage

### Running the program
Type `./t-chat` followed by your port number, the other user's hostname (typically their computer name), and then their port number.

Example:
```
./t-chat 4000 bobs-pc 5000
```

The other user would then have to do the reverse, typing your hostname, and your port number as the destination port number.
```
./t-chat 5000 joes-pc 4000
```
Now, you are able to communicate through the terminal.

You can also allow for loopback communication by specifying `localhost` as the other user's hostname and by providing the same source and destination port number.
```
./t-chat 4000 localhost 4000
```

### Exiting the program
To exit the chat, type in `!` press Enter. Note that only one user needs to do this, as both chat sessions will end upon exiting.
