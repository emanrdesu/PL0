Compiler and VM for programming language PL0 implemented with custom instruction set architecture.

prints everything to stdout
compile with "gcc compiler.c"
usage: ./a.out [-l] [-a] [-v] prog (in any order)

example: ./a.out -a -l myProg
myProg must be in the same directory as a.out
if myProg doesn't exist as a file in the current dir, it will cause a segmentation fault


errors and the code that causes them in file called "errors"
format ::= error_code " description " code_that_triggers_it

test input program named "input"