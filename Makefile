.phony all:
all: MFS

MFS: MFS.c
	gcc MFS.c -lpthread -lbsd -o MFS

.PHONY clean:
clean:
	-rm -rf *.o *.exe
