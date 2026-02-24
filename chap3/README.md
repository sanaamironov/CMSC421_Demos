# CMSC 421 — Chapter 3 (Processes) Demo Pack

This folder contains **demo-ready, production-quality** C programs for in-class demonstrations of Linux processes.

## Build
```bash
make
```

## Run (examples)
```bash
# 1) Observe process metadata in /proc
./01_proc_identity

# 2) Observe process memory layout + /proc/<pid>/maps
./02_memory_layout

# 3) fork() basics (two return paths)
./03_fork_basics

# 4) Copy-on-write behavior after fork
./04_fork_cow

# 5) fork() + exec() (PID stays, image replaced)
./05_fork_exec

# 6) Zombie demo (without wait) and fix (with wait)
./06_zombie_demo --no-wait
./06_zombie_demo --wait

# 7) Signals: SIGINT handler + SIGTERM graceful exit
./07_signals

# 8) Scheduling/context-switch pressure (observe with top/vmstat/pidstat)
./08_context_switch_pressure --cpu 6 --io 2 --seconds 40
# in another terminal:
#   kill -TERM <pid>
#   kill -KILL <pid>   (cannot be caught)
```

## Notes for instructors
- All programs print the PID and enough context to connect output to `ps`, `/proc`, `pmap`, and `strace`.
- Most demos intentionally pause (sleep) to give you time to inspect the process from another terminal.
- Designed for **Linux** (Debian/Ubuntu). Works on WSL2 as well.

## Helpful observation commands
```bash
ps -eo pid,ppid,state,comm --forest | head -n 30
top
htop
pstree -p | head -n 40

cat /proc/<pid>/status | head -n 40
cat /proc/<pid>/maps | head -n 40
pmap <pid> | head -n 40

strace -f -e trace=process,signal ./05_fork_exec
```
