# N-Shell

N-Shell is a minimal Unix shell written in C.

I built this project to demonstrate my understanding of **low-level programming** and some of the core **Linux/Unix operating system concepts** that shells rely on, including processes, file descriptors, pipes, signals, and system calls.

## Features

* Execute external commands with `fork()` and `execvp()`
* Built-in `cd` and `exit` commands
* Input redirection with `<`
* Output redirection with `>`
* Append redirection with `>>`
* Command pipelines using `pipe()` and `dup2()`
* `SIGINT` handling so `Ctrl+C` stops the running command without closing the shell
* Process synchronization using `waitpid()`

## Example

```bash
N-shell> echo hello > file.txt
N-shell> echo world >> file.txt
N-shell> cat file.txt
hello
world

N-shell> ls | grep ".c"

N-shell> sleep 10
^C
N-shell>
```

## What This Project Demonstrates

N-Shell focuses on the low-level mechanisms behind a Unix shell:

* Process creation and execution
* Parent and child processes
* File descriptors
* Standard input and output
* Inter-process communication with pipes
* I/O redirection
* Unix signals
* Linux/POSIX system calls

This is not intended to replace Bash or Zsh. It is a small systems-programming project built to better understand how shells interact with the operating system.



## Download

Precompiled Linux binaries are available in the **Releases** section.
