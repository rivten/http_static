#!/bin/bash

# NOTE(hugo) : I would like to use -pedantic-errors but anonymous structs are not a thing in C++11
# whereas it is in C11... who knows.
# NOTE(hugo) : I had to delete -Wcast-qual -Wshadow to compile stb libs
~/dev/ctime/ctime -begin compil_timings.ctm

CommonFlags="-g -std=c++11 -Werror -Wno-null-dereference -Wall -Wextra -Wcast-align -Wmissing-noreturn -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wno-old-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-promo -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option -Wno-write-strings -Wno-absolute-value -Wno-cast-align -Wno-unused-parameter -lm"

CommonIncludes="-I ../../rivten/ -I ../../stb/"
CommonLinks="-l SDL2 -l SDL2_ttf -l SDL_net"

clang++ $CommonFlags $CommonIncludes static_serv.cpp $CommonLinks -o ../build/http_serv

~/dev/ctime/ctime -end compil_timings.ctm
