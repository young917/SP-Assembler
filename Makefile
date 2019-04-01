CC = gcc

INC = 20171697.h variable.h
CFLAGS = -g
TARGET = 20171697.out
OBJECTS = Managing_String.o assembler.o 20171697.o

$(TARGET) : $(OBJECTS)
			  $(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

Managing_String.o : $(INC) Managing_String.c
			  $(CC) $(CFLAGS) -c -o Managing_String.o Managing_String.c

assembler.o : $(INC) assembler.c
			  $(CC) $(CFLAGS) -c -o assembler.o assembler.c

20171697.o : $(INC) 20171697.c
			  $(CC) $(CFLAGS) -c -o 20171697.o 20171697.c

clean:
	-rm -f $(OBJECTS) $(TARGET)
