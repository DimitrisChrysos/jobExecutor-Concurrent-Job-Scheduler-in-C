# 🧵 jobExecutor: Concurrent Job Management System in C (Spring 2024)

A system programming project that implements a job execution environment in C, using `fork()`, `exec*()`, signals, and named pipes (FIFOs). The system allows users to submit shell commands for execution with controlled concurrency, manage job queues, and extend functionality via bash scripts.

## 🔧 Components

- **jobExecutorServer**:  
  Manages job queue, executes jobs using child processes, and handles concurrency limits.
  
- **jobCommander**:  
  Frontend client used to issue commands such as:
  - `issueJob <job>`: Submit a new job.
  - `setConcurrency <N>`: Set max number of concurrent jobs.
  - `stop <jobID>`: Cancel or terminate a job.
  - `poll [running|queued]`: View running or pending jobs.
  - `exit`: Gracefully shutdown the server.

## 📂 Structure

- Written in C (or C++) for Linux.
- Inter-process communication via named pipes (mkfifo).
- Server uses `SIGCHLD` signal to track job termination.
- Unique job IDs (`job_XX`) and queue management.
- Includes:
  - `jobCommander.c`
  - `jobExecutorServer.c`
  - `Makefile`

## 📜 Bash Scripts

- `multijob.sh <file1> <file2> ...`:  
  Reads jobs from input files (one job per line) and submits them all.
  
- `allJobsStop.sh`:  
  Stops all active and queued jobs using the `stop` command.

## ⚙️ Compilation & Execution

1. Compile using:
   ```bash
   make
   ```

2. Submit jobs like:
   ```bash
   ./jobCommander issueJob "ls -l"
   ./jobCommander setConcurrency 4
   ./jobCommander poll running
   ./jobCommander stop job_2
   ./jobCommander exit
   ```

## 🧠 Design Notes

- Designed for robustness and modularity with separate compilation.
- Flow control through dynamic concurrency handling.
- Uses file `jobExecutorServer.txt` to manage server state.
- Implements signal handling and non-blocking I/O for responsiveness.

---

Project for: **System Programming – Spring 2024**  
Course: Κ24 – Προγραμματισμός Συστήματος
