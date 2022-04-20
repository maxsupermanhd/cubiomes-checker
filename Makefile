.PHONY: all clean

CC=gcc
CFLAGS=-c -Wall -O3 -Icubiomes/ -fwrapv
LDFLAGS=-Lcubiomes/ -lcubiomes -lm -fwrapv
SOURCES=find.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=find

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f a.out $(OBJECTS) $(EXECUTABLE)