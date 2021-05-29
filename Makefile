.PHONY: clean run runed

SHADERS			:= $(wildcard src/shaders/*.glsl)
SHADERS_PREPROC := $(patsubst src/shaders/%.glsl, build/tmp/shaders/%.glsl, $(SHADERS))

build/tmp/shaders/%.glsl: src/shaders/%.glsl $(wildcard src/shaders/*.h)
	@mkdir -p build/tmp/shaders
	cpp -P -undef $< > $@

all: update_ide $(SHADERS_PREPROC)
	ninja
	build/pack_shaders build/tmp/shaders build/shaders.bin

clean:
	rm -Rf build

run: all
	cd build && ./game

rundev: all
	cd build && ./devgame

update_ide:
	@echo "*** Update IDE project files... ***"
	rm -Rf 4MBJam.files
	touch 4MBJam.files
	find src/ -name "*.glsl" >> 4MBJam.files
	find src/ -name "*.hpp" -or -name "*.cpp" -or -name "*.h" -or -name "*.c" >> 4MBJam.files
	find extern/ -name "*.hpp" -or -name "*.cpp" -or -name "*.h" -or -name "*.c" >> 4MBJam.files
	@echo
