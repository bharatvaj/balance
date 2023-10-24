#!/bin/sh

make libbook.so hotbook.c && kill -3 $(pgrep hot)
