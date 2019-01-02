default: gameofstools

gameofstools: gameofstools.c gameofstools.h
	gcc -g -lMLV gameofstools.c -o gameofstools
