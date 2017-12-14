SRC = $(wildcard *.cpp *.c)
OBJ = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SRC)))

INC = -I. -I../include
CXXFLAGS = -g -Wall -std=c++14
TARGET = client

%.o: %.cpp
	@echo "building "$@
	g++ $(INC) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	gcc $(INC) -c $< -o $@

$(TARGET): $(OBJ)
	g++ $^ -o $@

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f $(TARGET)
