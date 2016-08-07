.PHONY: all clean

NAME	  = uuid
VERSION  := $(shell git describe --always --dirty --match v* | sed 's/^v//')
PLATFORM := $(shell uname -s)
CFLAGS    = -g -fPIC -Wall -Wextra -O2 -lcrypto
LDFLAGS   = -shared
DIST_DIR  = dist
SOURCES  := $(wildcard src/*.c)

ifeq ($(PLATFORM), Darwin)
	SO_EXT = dylib
else
	SO_EXT   = so
	LDFLAGS += -luuid
endif

all: $(DIST_DIR)/$(NAME)-$(VERSION).$(SO_EXT)

clean:
	$(RM) -r $(DIST_DIR)

print-%:
	@echo '$*=$($*)'

$(DIST_DIR)/$(NAME)-$(VERSION).$(SO_EXT): $(SOURCES)
	mkdir -p $(DIST_DIR)
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
