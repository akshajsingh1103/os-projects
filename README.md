# 🧠 Operating Systems Mini-Projects

This repository contains a collection of fundamental operating system component implementations, developed as part of coursework and independent learning. Each project focuses on simulating real-world OS-level functionalities using C and POSIX APIs, aimed at deepening understanding of process management, memory handling, concurrency, and system-level I/O.

## 📂 Projects Overview

### 1. SimpleScheduler
Implements a basic **round-robin process scheduler** in C. It simulates process management by scheduling dummy tasks with time slices and context switching behavior. Helps build intuition about CPU scheduling algorithms and process queues.

### 2. Multithreader
A lightweight **threaded task manager** using **POSIX threads (pthreads)**. Demonstrates creation, synchronization, and management of multiple concurrent threads, along with mutex usage for critical section control.

### 3. SimpleLoader
A custom-built **ELF (Executable and Linkable Format) loader** for 32-bit Linux executables. Mimics how an OS loader reads ELF headers, maps sections into memory, and transfers control to the entry point. Great for understanding binary execution at a low level.

### 4. SimpleShell
A basic command-line **shell interpreter** with support for executing user commands, handling background processes, and built-in commands like `cd` and `exit`. Simulates Unix-like shell behavior using `fork`, `execvp`, and `waitpid`.

### 5. SimpleSmartLoader
A more advanced ELF loader that improves upon `SimpleLoader`, adding smarter handling of segments, permissions, and memory mapping strategies — simulating a real-world dynamic linker more closely.

---

## 🛠️ Tech Stack

- **Language:** C
- **System APIs:** POSIX, Linux Syscalls
- **Tools:** GCC, Makefile, gdb, valgrind

---

## 📌 Note

These projects are academic and learning-focused implementations intended to simulate operating system behaviors. They are simplified by design and not meant for production use.
