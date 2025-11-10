# TODOs

- Move temporary files into a unique temporary directory (use `mkdtemp`/`mkstemp`).
- Add unit tests for parser and executor logic.
- Improve handling of escape sequences and nested quoting in the lexer.
- Add a small suite of integration tests that exercise pipelines, redirections, and background jobs.
- Audit and tighten memory error paths to eliminate leaks on all failure branches.
- Consider swapping manual line-editing for a well-tested lightweight library (e.g. linenoise) to reduce maintenance burden.

