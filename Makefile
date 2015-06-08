.PHONY: all clean

NAME	  = uuid
PLATFORM := $(shell uname -s)
CFLAGS    = -g -fPIC -Wall -Wextra -O2
LDFLAGS   = -shared
DIST_DIR  = dist
SOURCES  := $(wildcard src/*.c)

ifeq ($(PLATFORM), Darwin)
	SO_EXT = dylib
else
	SO_EXT   = so
	LDFLAGS += -luuid
endif

all: $(DIST_DIR)/$(NAME).$(SO_EXT)

clean:
	$(RM) -r $(DIST_DIR)

print-%:
	@echo '$*=$($*)'

$(DIST_DIR)/$(NAME).$(SO_EXT): $(SOURCES)
	mkdir -p $(DIST_DIR)
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
