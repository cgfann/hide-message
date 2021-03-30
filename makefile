#
# Nirre Pluf 
# 3/25/2021
# CSC 250 Lab 6

EXECUTABLE = lab6
COMPILE_FLAGS = -ansi -pedantic -Wall

$(EXECUTABLE): lab6.o get_wav_args.o
	gcc -o $(EXECUTABLE) lab6.o get_wav_args.o 

lab6.o: lab6.c get_wav_args.h 
	gcc $(COMPILE_FLAGS) -c lab6.c

get_wav_args.o: get_wav_args.h get_wav_args.c
	gcc $(COMPILE_FLAGS) -c get_wav_args.c

tidy: $(SOURCE)
	clang-tidy -checks='*' lab6.c -- -std=c99

clean:
	rm -rf $(EXECUTABLE) *.o

