.PHONY: all clean

CC                        := gcc
CFLAGS                    := -Wall -g -std=gnu99
CPPFLAGS                  :=
LDFLAGS                   :=
LIBS                      :=

INCLUDE_DIR               := include
SOURCE_DIR                := src
BINARY_DIR                := bin

DUMMY_FILE_GENERATOR_PROG := dfgen
DUMMY_FILE_GENERATOR_SRCS := main.c utils.c chunk.c genfile.c
DUMMY_FILE_GENERATOR_OBJS := $(patsubst %.c,%.o,$(DUMMY_FILE_GENERATOR_SRCS))

all: build_dummy_file_generator

build_dummy_file_generator:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(addprefix -I,$(INCLUDE_DIR)) -c $(addprefix $(SOURCE_DIR)/,$(DUMMY_FILE_GENERATOR_SRCS))
	mv *.o ./$(BINARY_DIR)/
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $(BINARY_DIR)/$(DUMMY_FILE_GENERATOR_PROG) $(addprefix $(BINARY_DIR)/,$(DUMMY_FILE_GENERATOR_OBJS))

clean:
	rm -rf $(BINARY_DIR)/$(DUMMY_FILE_GENERATOR_PROG)
	rm -rf $(BINARY_DIR)/$(DUMMY_FILE_GENERATOR_OBJS)
