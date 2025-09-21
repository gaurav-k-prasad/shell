# known bugs: 

1. There exists problems with SIGWINCH Signal and when user has written input in the terminal (not pressed enter yet) and resizes the terminal. the input is reprinted multiple times.
2. nested quotes echo "hello 'world \n' world' -> expected: 'hello 'world \n' world'
