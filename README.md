### Work in progress
The fronted part is currently under development. The compiler doesn't output any code. 

What has been finished and tested:
  - The lexer and parser. 
  
Under development:
  - Name identification.
  - Type checking.

### Tests

To run all the tests, launch this script on a Linux machine:

```$ ./run_tests.sh```

To test the parsing algorithm, pass a P4 source file to *ashp4c* as argument - there should be no errors. Edit the P4 code and introduce a mistake somewhere, then the compiler should be able to detect it and report it.

For example, this code fragment

```
  apply {
    a(8w15,);
  }
```
gives the error (note the comma in the example above)

```
$ ./ashp4c testdata/action-param1.p4
testdata/action-param1.p4:5:16: error: an argument was expected, got ')'.
```

