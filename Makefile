#application name here
TARGET=tic_tac_toe


#compiler
CC=gcc

#flags
CFLAGS=-Wall

PTHREAD=-pthread

#gtk
GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

# linker
LD=gcc
LDFLAGS=$(PTHREAD) $(GTKLIB) -export-dynamic

OBJS= game.o

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)
	

game.o: src/TIC_TAC_TOE_LAN.c
	$(CC) -c $(CCFLAGS) src/TIC_TAC_TOE_LAN.c $(GTKLIB) -o game.o
	
clean:
	rm -f *.o $(TARGET)
