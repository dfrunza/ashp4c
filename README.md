## This is a work in progress

The fronted part - parsing and type checking, is currently under development. At this stage the compiler doesn't output any code. 

What has been finished and tested:
  - The lexer and parser. 
  
Under development:
  - Name identification and type checking.

## Tests

To run all the tests, launch this script on a Linux machine:

```$ ./run_tests.sh```

To test the parsing algorithm, edit a P4 source file and introduce a mistake somewhere. The compiler should be able to detect it and report it.

For example, this code fragment:

```
  apply {
    a(8w15,);
  }
```
gives the error (note the comma in the example above):

```
$ ./ashp4c testdata/action-param1.p4
testdata/action-param1.p4:5:16: error: an argument was expected, got ')'.
```

