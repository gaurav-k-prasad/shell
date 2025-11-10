# Known issues and bugs

1. Quoting edge cases: nested or malformed quotes may produce unexpected tokenization results. Example observed: `echo "hello 'world \n' world'` — expected: `'hello 'world \n' world'` but lexer currently misparses nested quoting.

2. Escape sequence handling: escape sequence handling in the input lexer may not cover all terminal-emitted sequences; some exotic key combinations can be misinterpreted.

3. Temporary file collisions: temporary file usage for external integrations uses fixed filenames and may collide if multiple instances run concurrently. Use `mkstemp` or unique temporary directories to avoid collisions.

4. Memory leaks on error paths: some error paths may leak heap memory; a thorough valgrind/memcheck sweep is recommended and error-path cleanup should be tightened.

