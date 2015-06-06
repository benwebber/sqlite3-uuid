.PHONY: all clean

NAME	  = uuid
PLATFORM := $(shell uname -s)
CFLAGS    = -fPIC -Wall -Wextra -O2 -c
LDFLAGS   = -shared
BUILD_DIR = build
DIST_DIR  = dist
SOURCES  := $(wildcard src/*.c)
OBJECTS  := $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

ifeq ($(PLATFORM), Darwin)
	SO_EXT = dylib
else
	SO_EXT = so
endif

all: $(DIST_DIR)/$(NAME).$(SO_EXT)

clean:
	$(RM) -r $(BUILD_DIR) $(DIST_DIR)

print-%:
	@echo '$*=$($*)'

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o "$@" $<

$(DIST_DIR)/$(NAME).$(SO_EXT): $(OBJECTS)
	mkdir -p $(DIST_DIR)
	$(CC) $(LDFLAGS) -o $@ $<
