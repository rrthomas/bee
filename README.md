# Bee

by Reuben Thomas <rrt@sc3d.org>  
https://github.com/rrthomas/bee  

Bee is a simple virtual machine designed for the Forth language. It uses
word-based threaded code. An I/O library is implemented.

Bee’s small instruction set is easy to implement, yet also easy to use to
write a naive Forth compiler with reasonable performance.

This package comprises an implementation in ISO C99 using POSIX APIs.

The package is distributed under the GNU General Public License version 3,
or, at your option, any later version.

THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
RISK.


## Installation and compatibility

Bee should work on any POSIX-1.2001-compatible system. Bee has been tested
on x86_64 GNU/Linux with GNU C.

Reports on compatibility are welcomed.


### Building from a release tarball

help2man is required to build from source. For building from git, see below.

To build from a release tarball, run

`./configure && make && make check`


### Building from git

The GNU autotools are required: automake, autoconf and libtool.
[Gnulib](https://www.gnu.org/software/gnulib/) is also used, with a
third-party `bootstrap` module; these are installed automatically.

To build from a Git repository, first run

```
./bootstrap
```

Then see "Building from source" above.


## Use

Run `bee` (see `bee --help` for documentation).


## Documentation

Sorry, there’s only the source code at present.


## pForth

[pForth](https://github.com/rrthomas/pforth) is an ANSI Forth compiler that
runs on Bee.


## Running Bee object files

The C implementation of Bee allows a hash-bang line to be prepended to an object file, so that they can be run directly. A suggested line is:

```
#!/usr/bin/env bee
```


## Bugs and comments

Please send bug reports (preferably as [GitHub issues](https://github.com/rrthomas/bee/issues))
and comments. I’m especially interested to know of portability bugs.
