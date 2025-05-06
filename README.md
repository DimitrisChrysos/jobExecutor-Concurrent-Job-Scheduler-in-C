# 🧵 jobExecutor: Concurrent Job Management System in C (Spring 2024)

A system programming project that implements a job execution environment in C, using `fork()`, `exec*()`, signals, semaphores, and named pipes (FIFOs). The system allows users to submit shell commands for execution with controlled concurrency, manage job queues, and extend functionality via bash scripts.

### Note:
- Check report.txt and requirements.pdf for a more detailed explanation.

---

### 🔧 Components Overview

### jobCommander
Acts as the command-line interface for the user. It:
- Detects if the server is active and starts it if not.
- Sends commands to the server through FIFO pipes.
- Uses signals (`SIGUSR1`, `SIGUSR2`) to coordinate command transfer.
- Waits for and prints server responses.

### jobExecutorServer
Handles:
- Queueing of jobs (waiting/running).
- Execution via `fork()`/`execvp()`.
- Communication with jobCommander using FIFOs.
- Signal handling for job lifecycle (`SIGCHLD`) and command execution (`SIGUSR1`, `SIGUSR2`).

## 📂 File Structure

- `jobCommander.c`: Handles user interaction and communication.
- `jobExecutorServer.c`: Manages job queue, executes commands, signals, and process cleanup.
- `queue.c/h`: Custom queue and job triplet structures and logic.
- `ServerCommands.c/h`: Functions for job management (issue, stop, poll, etc.).
- `Makefile`: For compilation.
- `multijob.sh`: Batch job submission script.
- `allJobsStop.sh`: Stops all running and queued jobs.

## ⚙️ Compilation & Execution

### Build
```bash
make clean
make
```

### Example usage
```bash
Run the tests in bash
```

## 📜 Bash Scripts

### multijob.sh
- Reads job files line-by-line and prepends each line with `./jobCommander issueJob`.
- Submits all jobs in order.

### allJobsStop.sh
- Fetches all `queued` and `running` job IDs.
- Iteratively calls `./jobCommander stop <jobID>` to terminate them.

## 🧠 Design Specifics & Notes

- Custom queue structure implemented in `queue.c/h`.
- Commands are tokenized and handled via a `commands()` dispatcher.
- Server uses `SIGCHLD` + `waitpid(WNOHANG)` to handle completed jobs and immediately execute the next in queue.
- Commands like `issueJob`, `stop`, `poll`, and `exit` have distinct internal handling functions.
- Communication uses dual named pipes for reading and writing, with signaling to synchronize actions and semaphore support for multi-argument command packaging.

## 🧪 Test Notes & Known Behaviors

- `test_sh_scripts_2.sh` was edited to fix a command (`setConcurrency` call was incorrect).
- In `test_sh_scripts_1.sh`, execution continues up to a point before crashing—likely due to internal limitations or bugs.
  - It's recommended to run `test_sh_scripts_2.sh` **before** `test_sh_scripts_1.sh`.
- In `test_jobExecutor_6.sh`, due to immediate scheduling on `SIGCHLD`, small output differences may appear compared to expected output.

---

Project for: **System Programming – Spring 2024**  
Course: Κ24 – Προγραμματισμός Συστήματος  
Author: Δημήτριος Χρυσός
