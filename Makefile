SRC_FILES := $(shell find ./src -name "*.c")
OBJ_FILES := $(patsubst %.c,%.o,$(SRC_FILES))

TARGET := ./bin/main.bin
LIBS := -lm `sdl2-config --libs` -lGL -lGLEW
FLAGS := -g -DNANOVG_GLEW

all: $(OBJ_FILES)
	gcc $(FLAGS) $(OBJ_FILES) -o $(TARGET) $(LIBS)

run_main: all
	$(TARGET)

%.o: %.c
	gcc -c $(FLAGS) $^ -o $@ $(LIBS)


clean:
	rm $(OBJ_FILES) $(TARGET)