CC = clang
CFLAGS = -Wall -Wextra -Wshadow -std=c11 -fPIC -O3
LINK_FLAGS = -pthread

DEBUG ?= 1
DISABLE_ASSERT ?= 0
DEBUG_DISABLE_PRINT ?= 0

ifeq ($(DEBUG),1)
    CFLAGS += -DDEBUG
endif

ifeq ($(DISABLE_ASSERT),1)
    CFLAGS += -DDISABLE_ASSERT
endif

ifeq ($(DEBUG_DISABLE_PRINT),1)
    CFLAGS += -DDEBUG_DISABLE_PRINT
endif

ROOT_SRCS := $(wildcard *.c)  
SUB_SRCS := C-Thread-Pool/thpool.c
SRCS	:= $(ROOT_SRCS) $(SUB_SRCS)

ROOT_HEADERS := $(wildcard *.h) 
SUB_HEADERS := C-Thread-Pool/thpool.h
HEADERS	:= $(ROOT_HEADERS) $(SUB_HEADERS)

INCLUDES = -I. -I C-Thread-Pool

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = libchess.so.1

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -shared -Wl,-soname,libchess.so.1 -o $@ $(OBJS) $(LINK_FLAGS)
	ln -sf $@ libchess.so

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(TARGET) libchess.so

clean-precompiled:
	rm -f *.gch

clean-deps:
	rm -f $(DEPS)

clean-all: clean clean-precompiled clean-deps

.PHONY: all clean clean-precompiled clean-deps clean-all