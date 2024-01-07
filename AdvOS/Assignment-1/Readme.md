*xv6 "pb" Program README*

Description
The "pb" program, short for "pipebench," is a userspace program implemented in a single file named pb.c. It resides within the user subdirectory of the xv6-riscv-f23 project. This program is designed to benchmark data transfer across a pipe between parent and child processes within the xv6 operating system.

Implementation Details
The "pb" program performs the following tasks:

Pipe Creation: Utilizes the pipe system call to create a pipe.
Process Forking: Forks a child process.
Parent Process Actions:
Utilizes the uptime() system call to record the current time.
Receives 10 MB of data from the child process.
Validates the received data for accuracy; terminates with an error message if any byte is incorrect.
Uses the uptime() system call again to calculate and print the elapsed number of ticks for data transfer.
Uses wait() to wait for the child process before exiting.
Benchmarking Information

As a baseline, on the author's MacBook, transferring 10 MB across a pipe using the "pb" program takes approximately 100-150 clock ticks, varying based on compiler flags used.
