# my_watch

**Description:**

This is a repository for my system programming class project (2022).

`mon_watch` is a C program that mimics the behavior of the `watch` command in UNIX systems. It executes a specified command repeatedly until interrupted by the user. The interval between executions and the command itself are provided as command-line arguments.

**Features:**

- Utilizes POSIX signals for handling interruptions and timing.
- Forks a child process to execute the specified command.
- Monitors the child process and kills it if it exceeds the specified time interval or if interrupted by the user.
- Handles failures gracefully, providing meaningful error messages and appropriate exit codes.
- Prevents the program from hanging or entering into a busy wait loop.

**Usage:**

```
./mon_watch INTERVAL COMMAND [ARG]
```

- `INTERVAL`: The time interval in seconds between each execution of the command.
- `COMMAND [ARG]`: The command to be executed along with its arguments.

**Exit Codes:**

- `0`: Success (terminated by SIGINT).
- `1`: System call failure in the main program.
- `2`: Incorrect arguments provided.
- `4`: Unable to execute the specified command.
- `8`: Child process failed repeatedly.

**Compilation:**

```
cc -Wall -Wextra -Werror mon_watch.c -o mon_watch
```

**Run:**

```
./mon_watch INTERVAL COMMAND [ARG]
```

**Testing:**

To run the provided tests:

```
make test
```
