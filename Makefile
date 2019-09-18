.PHONY: all clean

CC                        := gcc
CFLAGS                    := -Wall -g -std=gnu99
CPPFLAGS                  :=
LDFLAGS                   :=
LIBS                      :=

INCLUDE_DIR               := include
SOURCE_DIR                := src

DUMMY_FILE_GENERATOR_PROG := dfgen
DUMMY_FILE_GENERATOR_SRCS := main.c utils.c
DUMMY_FILE_GENERATOR_SRCS := $(addprefix $(SOURCE_DIR)/,$(DUMMY_FILE_GENERATOR_SRCS))
DUMMY_FILE_GENERATOR_OBJS := $(patsubst %.c,%.o,$(DUMMY_FILE_GENERATOR_SRCS))

all: $(DUMMY_FILE_GENERATOR_PROG)

$(DUMMY_FILE_GENERATOR_PROG): $(DUMMY_FILE_GENERATOR_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(addprefix -I,$(INCLUDE_DIR))-o $@ $^

$(DUMMY_FILE_GENERATOR_OBJS): $(DUMMY_FILE_GENERATOR_SRCS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -rf $(DUMMY_FILE_GENERATOR_PROG)
	rm -rf $(DUMMY_FILE_GENERATOR_OBJS)
