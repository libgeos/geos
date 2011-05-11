#!/bin/sh

# tweak the number till you get a few lines of output and a segfault..
ulimit -v $((1024*18))

./memleak_mp_prep
