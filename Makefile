CC ?= cc
GLSLC ?= glslc
FLAGS = -Wall -Wextra -Wpedantic -std=c11
LINK = -lvulkan -lSDL2 -lm

.PHONY: clean

shader.%.spv: example/shader.%
	$(GLSLC) $^ -o $@

demo: example/main.c shader.vert.spv shader.frag.spv
	$(CC) $< $(LINK) $(FLAGS) -o $@

clean:
	rm shader.vert.spv
	rm shader.frag.spv
	rm demo
