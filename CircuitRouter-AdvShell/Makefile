# Makefile, versao 2
# Sistemas Operativos, DEI/IST/ULisboa 2018-19

SOURCES = CircuitRouter-AdvShell.c ../lib/commandlinereader.c ../lib/vector.c
OBJS = $(SOURCES:%.c=%.o)
CC   = gcc
CFLAGS =-Wall -std=gnu99 -I../
LDFLAGS=-lm
TARGET = CircuitRouter-AdvShell

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LDFLAGS)
	 cd ../CircuitRouter-SeqSolver && make

CircuitRouter-AdvShell.o: CircuitRouter-AdvShell.c ../lib/commandlinereader.h ../lib/vector.h ../lib/timer.h
../lib/commandlinereader.o: ../lib/commandlinereader.c ../lib/commandlinereader.h
../lib/vector.o: ../lib/vector.c ../lib/vector.h
../lib/timer.o: ../lib/timer.h

$(OBJS):
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGET)
	
nuke:
	@echo Cleaning...
	rm -f *.o ../*.o



