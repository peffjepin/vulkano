CC ?= cc
GLSLC ?= glslc
LINK = -lvulkan -lSDL2 -lm

.PHONY: clean

shader.%.spv: example/shader.%
	$(GLSLC) $^ -o $@

demo: example/main.c shader.vert.spv shader.frag.spv
	$(CC) $< $(LINK) -o $@

clean:
	rm shader.vert.spv
	rm shader.frag.spv
	rm demo
