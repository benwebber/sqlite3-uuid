.PHONY: all clean test

NAME	  = uuid
VERSION  := $(shell git describe --tags --always --dirty --match v* | sed 's/^v//')
PLATFORM := $(shell uname -s)
CFLAGS    = -g -fPIC -Wall -Wextra -Wno-unused-parameter -O2 -lcrypto
LDFLAGS   = -shared
DIST_DIR  = dist
SOURCES  := $(wildcard src/*.c)

ifeq ($(PLATFORM),Darwin)
	SO_EXT           = dylib
	USE_COMMONCRYPTO = implicit
	LIB             := $(DIST_DIR)/$(NAME).$(VERSION).$(SO_EXT)
else
	SO_EXT      = so
	USE_OPENSSL = implicit
	LDFLAGS    += -luuid
	LIB        := $(DIST_DIR)/$(NAME).$(SO_EXT).$(VERSION)
endif

ifneq ($(USE_OPENSSL),)
CFLAGS += -DUSE_OPENSSL
endif

ifneq ($(USE_COMMONCRYPTO),)
CFLAGS += -DUSE_COMMONCRYPTO
endif

ifneq ($(USE_DEFAULT_ENTRY_POINT),)
CFLAGS += -DUSE_DEFAULT_ENTRY_POINT
endif

ifneq ($(DEBUG),)
CFLAGS += --coverage
endif

all: $(LIB)

clean:
	$(RM) -r $(DIST_DIR)
	$(RM) *.gcda *.gcno *.gcov

print-%:
	@echo '$*=$($*)'

test: $(LIB)
	tox -- --extension=$(LIB)

$(LIB): $(SOURCES)
	mkdir -p $(DIST_DIR)
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
