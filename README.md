
# LibCC

Is a header-only C++ library for Windows with a few utilities:

* LibCC::Format is a string formatter designed for fast performance and sane syntax
* LibCC::Log is a simple class that outputs log messages to a window, stdout, stderr, or a file. It's multi-thread friendly with a tab for each thread output.
* LibCC::Timer et al are classes that help with profiling / timing code.
* There are a bunch of windows API wrappers and helpers.

# TODO

* Portability; add a gcc(/other?) build environment?
** one part of this is using portable string conversion stuff.
** also, what's the best way to deal with typedef FormatX<TCHAR> Format
* try to operate on utf-8 by default instead of supporting stupid ANSI windows crap.
* Consider move ctors and modern C++ stuff
* fix test suite to use log & better output. it's so ugly
