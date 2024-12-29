## Project status

The compiler is under development and at this stage it doesn't output any code. The lexer and parser have been finished and tested, name indentification and type checking are work in progress.

## Building the compiler

Building the compiler executable requires a Linux OS and GCC. Change the current directory to the root of the source tree and launch the `build.sh` script.

## Tests

'run_tests.sh' will run 'ashp4c' on a series of test P4 programs located in the 'testdata' folder:

```$ ./run_tests.sh```

To test the parsing algorithm, edit a P4 source file and introduce an error somewhere. The compiler should be able to detect and report it:

```
  apply {
    a(8w15,);
  }
```
In this code fragment the error is caused by the trailing comma:

```
$ ./ashp4c testdata/action-param1.p4
testdata/action-param1.p4:5:16: error: an argument was expected, got ')'.
```

