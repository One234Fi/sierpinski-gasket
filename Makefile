CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-parameter -g -MMD
LDLIBS := -lglfw -lvulkan -lpthread -ldl -lX11 -lXxf86vm -lXrandr -lXi
LDFLAGS := 

SRCS := main.c vector.c
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS+=-DDEBUG -fsanitize=address
else 
	CFLAGS+=-DNDEBUG
endif
	
app.out: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS) $(LDFLAGS)

shaders/vert.spv:
	glslc -fshader-stage=vertex shaders/shader.vert.glsl -o shaders/vert.spv

shaders/frag.spv:
	glslc -fshader-stage=fragment shaders/shader.frag.glsl -o shaders/frag.spv

shaders: shaders/vert.spv shaders/frag.spv

recompileShaders:
	glslc -fshader-stage=vertex shaders/shader.vert.glsl -o shaders/vert.spv
	glslc -fshader-stage=fragment shaders/shader.frag.glsl -o shaders/frag.spv

-include $(DEPS)

clean:
	rm -f app.out $(DEPS) $(OBJS)
