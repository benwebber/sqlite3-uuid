.PHONY: all clean test

NAME	  = uuid
VERSION  := $(shell git describe --always --dirty --match v* | sed 's/^v//')
PLATFORM := $(shell uname -s)
CFLAGS    = -g -fPIC -Wall -Wextra -Wno-unused-parameter -O2 -lcrypto
LDFLAGS   = -shared
DIST_DIR  = dist
SOURCES  := $(wildcard src/*.c)

ifeq ($(PLATFORM), Darwin)
	SO_EXT = dylib
	CFLAGS := $(CFLAGS) -Wno-deprecated-declarations
else
	SO_EXT   = so
	LDFLAGS += -luuid
endif

LIB := $(DIST_DIR)/$(NAME)-$(VERSION).$(SO_EXT)

all: $(LIB)

clean:
	$(RM) -r $(DIST_DIR)

print-%:
	@echo '$*=$($*)'

test: $(LIB)
	py.test --extension=$(LIB)

$(LIB): $(SOURCES)
	mkdir -p $(DIST_DIR)
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
