CC=gcc

INC        =
LDFLAGS    =
LDLIBS     =

CPPFLAGS   = 
CFLAGS     = -g -Wall -Wextra -O2

SOURCES    = rm-clones.c util.c sha256/sha256.c
OBJECTS    = $(SOURCES:.c=.o)
EXECUTABLE = $(shell basename `pwd`)

all: options $(SOURCES) $(EXECUTABLE)

options:
	@echo build options:
	@echo "CFLAGS	= $(CFLAGS)"
	@echo "CPPFLAGS = $(CPPFLAGS)"
	@echo "LDFLAGS	= $(LDFLAGS)"
	@echo "CC	= $(CC)"
		
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

.c.o:
	$(CC) -o $@ -c $(CPPFLAGS) $(CFLAGS) $<
		
install: all
	cp -f $(EXECUTABLE) /usr/local/bin/$(EXECUTABLE)

deinstall:
	rm -f /usr/local/bin/$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
