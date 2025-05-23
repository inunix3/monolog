# monolog

A simple, interpreted C-like programming language.

This was a school project, so the language is toyish and the interpreter is pretty simple.

Anyway, created just for fun.

```c
println("Hello, World!");

void fizzbuzz(int n) {
    if (n <= 0) {
        return;
    }

    for (int i = 1; i <= 100; ++i) {
        print($i + "... ");

        if (i % 15 == 0) {
            println("fizz buzz");
        } else if (i % 3 == 0) {
            println("fizz");
        } else if (i % 5 == 0) {
            println("buzz");
        } else {
            println("");
        }
    }
}

fizzbuzz(100);
```

## Features 

- Imperative
- Infix notation (`2 + 3 / 0`)
- Arithmetic (`+, -, *, /, %`)
- Logic (`!, ==, !=, <, >, <=, >=, &&, ||`)
- Basic branching statements (`if, else`)
- Loops (`while, for`)
- Block statements
- Functions
- Builtin types:
    - 64-bit signed integers (`int`)
    - Mutable, resizable strings (`string`)
    - Option types (`T?`)
    - Arrays (`[T]`)
- REPL

## Examples and documentation

Example programs are located in `examples/`.

Reference for the language you can find in `docs/`.

## Building

See documentation in `docs/`.

## License

Monolog is licensed under the [MIT License](LICENSE.md).
