## Project status

The compiler is under development and at this stage it doesn't output any code. 

What has been finished and tested:
  - The lexer and parser. 

Under development:
  - Name identification and type checking.

## Building the compiler

To build the `ashp4c` executable on a Linux OS, change the current directory to the root of the source tree and launch the `build.sh` script. The `gcc` C-compiler is required.

## Tests

To run all the tests, launch this script:

```$ ./run_tests.sh```

To test the parsing algorithm, edit a P4 source file and artificially introduce an error somewhere. The compiler then should be able to detect and report it.

For example, this code fragment:

```
  apply {
    a(8w15,);
  }
```
is throwing this error (note the comma):

```
$ ./ashp4c testdata/action-param1.p4
testdata/action-param1.p4:5:16: error: an argument was expected, got ')'.
```

