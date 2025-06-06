# 🧠 Operating Systems Projects – Grade 10/10

This repository contains three Operating Systems assignments developed in C, each graded **10/10** at the Technical University of Cluj-Napoca. The projects explore key low-level system programming concepts such as file parsing using raw system calls, advanced process/thread synchronization, and inter-process communication via shared memory and named pipes.

## 📁 Assignment Overview

### 🔧 Assignment 1: Custom File System Parser (`a1.c`)

Implements a Linux command-line utility to:
- Recursively list directory contents with support for filtering by file size (`size_smaller`) and file name prefix (`name_starts_with`).
- Parse a custom binary file format called **SF (Section File)** by interpreting a binary header and validating fields like:
  - `MAGIC`, `VERSION`, `NO_OF_SECTIONS`, `SECTION_TYPE`
- Extract lines from specific sections using byte-level offset calculations.
- Traverse directory trees and identify all valid SF files that contain only sections smaller than 1273 bytes.

📌 **System-level highlights**:
- Uses only **low-level POSIX system calls** (`open()`, `read()`, `lseek()`, `close()`, `opendir()`, `readdir()`) — no `fopen()` or `fscanf()`.
- Implements robust **binary parsing logic** (headers + sections) using manual byte offset navigation and validation.
- Handles edge cases like malformed files, invalid magic/version, or incorrect section configurations.

---

### 🔧 Assignment 2: Multi-Process & Multi-Thread Synchronization (`a2.c`)

Builds a complex **process tree (7 processes)** and implements **fine-grained thread synchronization** using:
- `pthread_create()` for thread management
- **POSIX semaphores** and **mutexes/condition variables** for synchronization
- Proper `fork()` process isolation and parent-child lifecycle control

📌 **Key constraints implemented**:
- Process P5 spawns **38 threads**, enforcing a strict limit of max **6 concurrent running threads** (true concurrency control, not creation cap).
- Implements **thread barrier semantics**: `T5.11` must terminate while exactly 6 threads are active.
- Cross-process synchronization between P6 and P7 threads:
  - `T7.6` must terminate before `T6.1` starts.
  - `T7.3` must start **only after** `T6.1` terminates.
- Process P6: `T6.4` starts before `T6.2`, and ends after it — demonstrating ordered thread dependency control.

📌 **Testing interface integration**: All processes and threads communicate with the evaluator using `init()` and `info(BEGIN/END, PID, TID)` hooks, ensuring synchronization constraints are externally verifiable.

---

### 🔧 Assignment 3: IPC with Shared Memory, Memory-Mapped Files, and Named Pipes (`a3.c`)

Implements a protocol-based **inter-process communication server** using:
- **POSIX named pipes (FIFOs)**: bidirectional communication with a tester client
- **POSIX shared memory** (`shm_open`, `ftruncate`, `mmap`) for data exchange
- **Memory-mapped files** for reading binary SF file content with performance in mind

📌 **Supported operations via command protocol**:
- `CONNECT`, `PING`, `CREATE_SHM`: Establish pipe communication and initialize shared memory region (2.4MB).
- `MAP_FILE`, `WRITE_TO_SHM`, `READ_FROM_FILE_OFFSET`: Efficient file I/O using memory mapping, no direct `read()` on files allowed.
- `READ_FROM_FILE_SECTION`: Extracts a number of bytes from a given section and offset in a SF-format file.
- `READ_FROM_LOGICAL_SPACE_OFFSET`: Calculates logical memory layout using 3072-byte alignment between sections, simulates virtual file mapping, and reads data at calculated logical offsets.

📌 **System-level constraints**:
- Must validate offsets and bounds before memory access.
- Penalized for fallback to `read()` instead of `mmap` access.
- Follows strict protocol format using string/number fields with special terminators (e.g., `#`), implementing a **custom low-level protocol parser**.

---

## 🛠 Technologies & Concepts

- **C (C89/C99 standard)**
- **Linux system programming**: file descriptors, directory traversal, memory-mapped I/O
- **POSIX threads & synchronization**: `pthread`, `sem_t`, `mutex`, condition variables
- **IPC mechanisms**: named pipes (FIFOs), shared memory, and memory-mapped files
- **Custom protocol implementation**: byte-wise parsing and encoding of messages

## 📘 Academic Context

These assignments were developed in the context of the **Operating Systems** course in Year 3 of my Computer Science degree. The focus was on mastering systems programming by solving real-world tasks with low-level control over execution and memory.

I am highly passionate about this field and consistently excelled in it — each of these assignments was awarded the **maximum grade of 10/10**.

---

## 🏅 Author

**Francesco Maxim**  
- Student @ UTCN, Faculty of Automation and Computer Science  
- Email: maaximfrancesco@gmail.com  
- GitHub: [maxim-francesco](https://github.com/maxim-francesco)  
- LinkedIn: [linkedin.com/in/francescomaxim](https://linkedin.com/in/francescomaxim)

---

> 💡 If you're passionate about systems programming, feel free to connect! I'm always open to collaboration and low-level development challenges.
