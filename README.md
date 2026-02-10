
# git-each

broadcasts commands to many local git repositories

## Installation

currently this program isn't packaged anywhere.  feel free to build from source.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja # -DCMAKE_INSTALL_PREFIX=${MY_PREFIX}
cmake ---build build
ctest --test-dir build # if gtest is installed with pkg-support on your system
cmake --install build
```
```
```

## Docs (man page)

### Options

`-d <DIR>` | `--directory <DIR>`

the base directory to search through for git repositories.

defaults to the current working directory.

`-s` | `--system`

the command is a system one instead of git oriented

`-D` | `--discover`

just search and print repositories (do not run an action)

`-j <N>` | `--jobs <N>`

the amount of threads to run while processing the commands

`-f <FORMAT>` | `--format <FORMAT>`

the format of the report after a command has finished

`-F <SCRIPT>` | `--file <SCRIPT>`

a script to execute instead of a running a command

`-I <INTERP>` | `--interpreter <INTERP>`

an interpreter to run the `SCRIPT`

by default uses `SHELL`

`--`

Interprets the rest of the arguments as part of the command to be run
