COMPILER=gcc 
COMPILERFLAGS=-Wall -pedantic -fsanitize=address -g -lrt -pthread -Wno-pointer-arith -fcommon
LDFLAGS=-pthread