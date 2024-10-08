CC=g++
CXXFLAGS=-g -Wall -Wextra -O0 -pthread
DEPS = Semaphore.h BoundedBuff.h
OBJ = babyyoda.o Semaphore.o BoundedBuff.o
BIN = babyyoda 

%.o: %.cpp $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BIN): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ 

.PHONY: clean

clean:
	rm -f *.o *~ $(BIN)
