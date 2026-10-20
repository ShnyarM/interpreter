# Interpreter

An interpreter for a self-made programming language written in C. Currently has support for the following:

- Variables
- Functions
- Dynamic type checking
- Arrays
- Strings
- Numbers (floating-point doubles)
- Command-line arguments
- Std output
- Built-in functions

Complete with automatic memory allocation for dynamic structures, and a garbage collector based on reference counting.

## Programs

At the moment, two example programs can be found in the [input/](input/) directory:

1. **Game of life**: An implementation of Conway's game of life. The number of generations it should run for is given as a command-line argument.

2. **JSON-like pretty printer**. The object to be pretty-printed is specified in the program.

Additional examples can be found in the [testing/](testing/) directory.

## Running the interpreter

The interpreter can be compiled using `make`. This will create an executable called `interpreter`, which can be executed in the following way:

```./interpreter [PROGRAM_FILE] [ARG1] [ARG2] ...```

## Testing

Test cases can be found under the [testing/](testing/) directory. These can be executed using the [python test script](testing/test.py).

```python3 testing/test.py```

This will go through all test cases and make sure that the output matches the output specified in the corresponding `.json` file.

Additonally, running:

```python3 testing/test.py --memory```

will check all test cases for memory leaks using `valgrind`.

Note that, due to the nature of reference counting, the interpreter **can contain memory leaks when defining cyclic structures**.
