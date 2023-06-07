# Lotus

## Summary

This is an interpreter written in C for the Lotus language (*currently WIP*). I want to achieve a multithreaded functional PL with optimizations for Pure code.

## Makefile

Installation is easy if you are on a Unix system.
For Windows users please get help.

### To build 

```bash
git clone https://github.com/SpanishInquisition49/lotus.git
cd lotus
make # Compile the project with the debug flag
```

### Usage
```bash
./lotus [source_code]
```

### Testing
```bash
make run_test # Run some test for the logic
make valgrind # Run valgrind
```

Currently only simple arithmetic expression are supported.