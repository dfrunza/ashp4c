## Project status

The compiler is under development and at this stage it doesn't output any code.
The lexer and parser have been finished and tested.
Work on name indentification and type checking is in progress.

## Building the compiler

Building the executable requires a Linux OS and the GCC compiler.
Change the current directory to the root of the source tree and launch the `build.sh` script.

## Tests

This script compiles a list of P4 programs and reports the PASS/FAIL status of each:

```$ ./run_tests.sh```

Edit a P4 source file and introduce an error somewhere. The parser should be able to detect it:

```
  apply {
    a(8w15,);
  }
```

In the above code fragment the error is caused by a trailing comma:

```
$ ./ashp4c testdata/action-param1.p4
testdata/action-param1.p4:5:16: error: an argument was expected, got ')'.
```

