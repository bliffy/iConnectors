#this Makefile is for g++ on linux or in mingw-64

ifeq ($(OS),Windows_NT)
	libs:=-lws2_32
else
	libs:=
endif
CC:=c++

all: tcptest

clean:
	rm tcptest

tcptest:
	$(CC) -Wall tcpconnectors.cc tcptest.cpp -o tcptest $(libs)

