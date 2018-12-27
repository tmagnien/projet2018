default: gameofstools

gameofstools: gameofstools.c gameofstools.h
	gcc -lMLV gameofstools.c -o gameofstools
