.PHONY: clean run runed

CXXFLAGS	= -Os

all:
	ninja

clean:
	ninja -t clean

run: all
	cd build && ./game

runed: all
	cd build && ./editor
