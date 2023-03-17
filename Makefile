CC ?= cc
GLSLC ?= glslc
FLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
LINK = -lvulkan -lSDL2 -lm

.PHONY: clean all

build/shader.%.spv: example/shader.%
	@mkdir -p build
	$(GLSLC) $^ -o $@

bin/hello_quads: build/main.o build/shader.vert.spv build/shader.frag.spv
	@mkdir -p bin
	$(CC) $< $(LINK) $(FLAGS) -o $@

build/main.o: example/main.c
	@mkdir -p build
	$(CC) -c $(FLAGS) $< -o $@

all: bin/hello_quads

clean:
	-rm -rf bin
	-rm -rf build
