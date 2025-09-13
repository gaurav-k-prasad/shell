# Tests required

## Test input parsing

1. echo " $path $ \$ "
2. echo " $ $SHELL \$ "
3. echo "$shell \$"
4. test: hello \ world
5. test: hello \n world
6. test: hello<bru
7. test: pipe <hello
8. test: pipe> hello
9. test: pipe|bp
10. test: '' and realted inputs
11. $INVALID$VALID
12. $VALID$VALID
13. $VALID$VALID$VALID
14. $VALID$INVALID$VALID
15. above 4 and similar with "" and ''
16. $PATH\ $PATH
17. cat < input | grep a | grep b > write && echo "hello world" ; echo "bruh done\n"

---

## Todos
