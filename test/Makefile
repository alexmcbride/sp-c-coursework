# Only the next 3 lines need to be edited - leave the rest!
TARGET = test 
DEPS = 
OBJ = test.o

CC=cc
CFLAGS=-I. -Wall -g -O0

all: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)




