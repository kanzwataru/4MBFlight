.PHONY: clean run runed

SHADERS			:= $(wildcard src/shaders/*.glsl)
SHADERS_PREPROC := $(patsubst src/shaders/%.glsl, build/tmp/shaders/%.glsl, $(SHADERS))

build/tmp/shaders/%.glsl: src/shaders/%.glsl $(wildcard src/shaders/*.h)
	@mkdir -p build/tmp/shaders
	cpp -P -undef $< > $@

all: $(SHADERS_PREPROC)
	ninja
	build/pack_shaders $(PWD)/build/tmp/shaders build/shaders.bin

clean:
	rm -Rf build

run: all
	cd build && ./game

rundev: all
	cd build && ./devgame
