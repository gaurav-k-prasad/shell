# Usage

Quick start

1. Ensure you have a C compiler (`gcc`) and `make` installed.
2. Build the shell:

```sh
make
```

3. Run the shell:

```sh
./gshell
```

Notes on running

- The shell expects to run on POSIX-compliant systems (Linux, macOS may work for many features).
- If you use the optional external integration, the helper script is expected to exist at `./AI/interface.py`. The shell runs normally without that script.

Common commands

- Change directory: `cd /path/to/dir`
- Print working directory: `pwd`
- Run a pipeline: `ls | grep txt`
- Redirect output: `cat < infile > outfile`
- Run in background: `sleep 10 &`
- Execute a script: `source script.sh`

Troubleshooting

- If terminal input behaves oddly after exit, ensure the program restored terminal settings; if not, run `stty sane` in your terminal to restore defaults.
- For permission or path issues when launching external commands, verify `PATH` and file executable bits with `ls -l` and `which`.
