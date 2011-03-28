.PHONY: all clean

all:
	make -C tools all
	make -C target all

clean:
	make -C tools clean
	make -C target clean
