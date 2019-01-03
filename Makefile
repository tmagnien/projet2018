default: gameofstools

gameofstools: gameofstools.c gameofstools.h
	gcc -g gameofstools.c -lMLV -o gameofstools
