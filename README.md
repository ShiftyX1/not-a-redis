# (not) a Redis

A toy implementation of a Redis-like server in C++.
This is a learning project for me to understand how Redis works internally. It is not intended for production use.

Thanks to [Build-your-own-X](https://github.com/codecrafters-io/build-your-own-x) repository for helping me get started with this project!


# Building

Requirements:
- A C++17 compatible compiler (e.g., g++, clang++)
- CMake 3.10 or higher
- Python 3.10 or higher (for the client)

```bash
cmake . && make
```

Now you can run the server:

```bash
./my_own_redis
```

# Client

In ```Build-your-own-X``` client and server were written on C/C++. But I decided to write a simple client in Python for easier testing and just because I like Python.

You can run the client like this:

```bash
uv run ./client.py

# Help
uv run ./client.py --help
```

# Server protocol

The server uses a simple binary protocol over TCP.
Each command is sent as:
- 4 bytes: total length of the command (excluding this length field)
- 4 bytes: number of arguments (N)
- For each argument:
  - 4 bytes: length of the argument (L)
  - L bytes: argument data

The server responds with:
- 4 bytes: total length of the response (excluding this length field)
- 4 bytes: status code (0 = OK, 2 = NX, 1 = ERR)
- Remaining bytes: response message (if any)

## Commands

Currently supported commands:
- `SET key value`: Sets the value for the given key.
- `GET key`: Gets the value for the given key.
- `DEL key`: Deletes the given key.


# Afterwords

I don't plan to make this project more complex, but I might add some features in the future. And I don't know if you would find this project useful, but feel free to use it as a learning resource or a starting point for your own projects!
Enjoy coding!
Love yourself!
Be happy! ☺️