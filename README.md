# Lambda

## Summary

This is an interpreter written in C for a custom language (currently WIP). I want to achieve a multithreaded functional PL with optimizations for Pure code.

## Build

To build 

```bash
git clone https://github.com/SpanishInquisition49/lambda.git
cd lambda
make
```

Usage
```bash
./lambda [source_code]
```

Currently only a primitive version of the Lexer (scanner) and a even simpler parser are implemented.

## Syntax

|  Grammar |
| - |
| let [identifier] = {expr} |
| fun [identifier]\([formals]) => {expr} |
| print {expr} |
| match {expr} with {expr} |
| nil |
| if({expr}) {expr} *[else {expr}]* |
