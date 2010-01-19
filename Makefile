.PHONY: all clean

all:
	make -C tools all
	make -C src all

clean:
	make -C tools clean
	make -C src clean
