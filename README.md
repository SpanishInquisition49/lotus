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
./lotus [source.lts]
```

### Testing

```bash
make check # Run some test for the logic
make valgrind # Run valgrind
```

### Configurations

The lotus interpreter can read the file ``~/.config/lotus/lotus.conf`` *(not created by default)*, below a list of options and their possible values

|Option|Possible values|Short description|
|------|:-------------:|:----------------|
|LOG_LEVEL|WARNING/ERROR/INFO|verbosity of errors|
|PRINT_REPORT|TRUE/FALSE|an overview of warnings and errors between every phase |

#### Default config

```txt
LOG_LEVEL=WARNING
PRINT_REPORT=FALSE
```

## Syntax

### Declarations

Every declaration extend the environment with a pair identifier value, also the declaration evaluate with the same value obtained evaluating the expression.

```js
let ide = exp;
//example
let x = 2+3; // add to the env x:=2 and yield 2
```

### Functions declarations

Functions are declared with the keyword *fun*, like normal declarations a function declaration extend the environment and yield the closure as a value.

```ml
fun ide([formals]) body
//example
fun double(x) x*2;
fun fact(x) {
    if(x==0)
        1;
    else
        x*fact(x-1);
}
```

### Assignments

Declarations binds identifiers with values, but this values are mutable, a value associated to an identifier can be changed with an assignment. An assignment yield as a result the new value.

```js
ide = exp;
//example
x = 5;
```

### Conditional statements

Like the majority of programming languages the *if/else* evaluate a condition and depending on the result execute a block or if present the other one. In addition to this the evaluation of a conditional statement yield the result of the executed block.

```js
if(condition)
    //block
[else]
    //block
//example
if(x%2==0)
    print "Even";
else
    print "Odd";

```

### Print statements

The print statement write on stdout the value obtained by evaluating the given expression. Every print yield a nil value

```js
print exp;
//example
print "Hello";
print x;
```

### Function calls

A function call is formally an expression, a call is formed by an identifier followed by the list of actual parameters (a list of expressions). **Note:** for now the return statement is unimplemented, but the default return value is the last yielded value from the body.

```js
ide([actual]);
//example
double(2);
```

### Expressions

#### Types

Lotus is a strongly and dynamic typed language, there are just four atomic types:

* $S$ String
* $N$ Numbers
* $B$ Booleans
* $F$ Functions
* $Nil$ (for now there is no usage for the nil type)

#### Operations

A lower priority operator are evaluated before other higher operators, functions call have the precedence above every other operators.

| Unary operator | Usage | Priority | Domain |
|----------------|:-----:|:--------:|:------:|
| Logical not    | !exp  | 2        | $B\to B$
| Minus          | -exp  | 2        | $N\to N$

| Binary operator      | Usage          | Priority | Domain |
|----------------------|:--------------:|:--------:|:------:|
| Multiplication       | exp * exp      | 3        | $N\times N\to N$
| Division             | exp / exp      | 3        | $N\times N\to N$
| Remainder            | exp % exp      | 3        | $N\times N\to N$
| Forwarding           | exp \|> call   | 3        | $\alpha\times F\to\alpha$
| String concatenation | exp + exp      | 4        | $S\times S\to S$
| Addition             | exp + exp      | 4        | $N\times N\to N$
| Subtraction          | exp - exp      | 4        | $N\times N\to N$
| Greater than         | exp > exp      | 5        | $N\times N\to B$
| Greater or equal     | exp >= exp     | 5        | $N\times N\to B$
| Lower than           | exp < exp      | 5        | $N\times N\to B$
| Lower or equal       | exp <= exp     | 5        | $N\times N\to B$
| Logical and          | exp and exp    | 6        | $B\times B\to B$
| Logical or           | exp or exp     | 6        | $B\times B\to B$
| Equality             | exp == exp     | 7        | $\alpha\times\alpha\to B$
| Not equality         | exp != exp     | 7        | $\alpha\times\alpha\to B$

#### Forwarding

Nested functions call can be written with the forward operator. The value on the left side of the operator will be used as the first argument of the right function call.

```ts
factorial(double(5))
// Is equivalent to
double(5) |> factorial()
// Or
5 |> double() |> factorial()
// If a function have multiple arguments
sum(5, y)
// Then you can write (all arguments in the call will be shifted by 1 position)
5 |> sum(y)
```

## TODO

* [ ] Add return statement
* [x] Add string concatenation
* [ ] Add arrays
* [ ] Add list
* [ ] Add pattern matching
* [ ] Fix garbage collector
* [ ] Add stack management
* [ ] Add the ability to read input from stdin
* [ ] Add the ability to format strings
