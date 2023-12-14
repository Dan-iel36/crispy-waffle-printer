.PHONY : all

all:  install compile build


compile:
	@gcc -c main.c -o main.o


build: compile
	@gcc main.o -o printer

install: build
	@chmod +x printer


clean:
	@rm -rf ./"*.o"
	@rm -rf printer
