Overview
This xv6 enhancement aims to significantly improve pipe performance by optimizing data transfer across the user/kernel boundary and implementing a custom memory copy routine.

File Modifications

Part A: Improved Pipe Efficiency
File: pipe.c
Objective: Optimizing data transfer across the user/kernel boundary in pipe operations.
Changes: Modified copyin() and copyout() functions to maximize byte transfer in each call.

Part B: Custom Memory Copy Routine
File: vm.c
Objective: Replacement of the default memmove() with a custom memory copy routine for improved performance.
Changes: Implemented a memory copy function operating on 64-bit chunks while handling unaligned addresses in compliance with strict aliasing rules using GCC's __may_alias__ attribute.

Performance Evaluation
Measurement and Analysis
Objective: To measure the impact of enhancements made in pipe.c and vm.c on pipe performance using the pb program.
Procedure:
Baseline measurement in unmodified xv6.
Testing xv6 with modified pipe.c.
Testing xv6 with customized memory copy routine in modified vm.c.
Measurement with both enhancements implemented.
Performance Metrics: Observed speedup reported in a comment within pipe.c.
