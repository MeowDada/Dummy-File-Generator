.PHONY: all clean

CC                        := gcc
CFLAGS                    := -Wall -g -std=gnu99
CPPFLAGS                  :=
LDFLAGS                   :=
LIBS                      :=

INCLUDE_PATH              := include


DUMMY_FILE_GENERATOR_PROG := dfgen
DUMMY_FILE_GENERATOR_SRCS := main.c
DUMMY_FILE_GENERATOR_OBJS := $(patsubst %.c,%.o,$(DUMMY_FILE_GENERATOR_SRCS))

all: %.o $(DUMMY_FILE_GENERATOR_PROG)

$(DUMMY_FILE_GENERATOR_PROG): $(DUMMY_FILE_GENERATOR_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -rf $(DUMMY_FILE_GENERATOR_PROG)
	rm -rf $(DUMMY_FILE_GENERATOR_OBJS)
