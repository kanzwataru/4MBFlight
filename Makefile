.PHONY: clean run runed

all:
	ninja

clean:
	ninja -t clean

run: all
	cd build && ./game

rundev: all
	cd build && ./devgame
