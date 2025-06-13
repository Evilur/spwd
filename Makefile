# Compiler and its flags
CC 	   ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wuninitialized \
-Wconversion -Wlogical-op -Wnull-dereference -Wduplicated-cond \
-Wredundant-decls -Wstrict-prototypes -Wmissing-declarations \
-Wunreachable-code -Wmissing-prototypes -O2
CFLAGS += -std=c99

# Prefix
PREFIX	    ?= /usr/local
BINPREFIX   ?= $(PREFIX)/bin
SHAREPREFIX ?= $(PREFIX)/share

# Directory with share files
SHAREDIR := $(SHAREPREFIX)/UniversalPause

# Set targets that do not create new files
.PHONY: clean install uninstall

# Build the C binary
spwd:
	$(CC) $(CFLAGS) spwd.c -o spwd

# Clean all compiled C binaries
clean:
	rm --force spwd

# Install the program to the system
install: spwd
	cp spwd $(BINPREFIX)

# Uninstall the program from the system
uninstall:
	rm $(BINPREFIX)/spwd
