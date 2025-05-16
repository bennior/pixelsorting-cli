CXX		:= gcc
CXX_FLAGS 	:= -Wall -Wextra -ggdb
OPTIMIZATION	:= -O3

BIN		:= bin
SRC		:= src
INCLUDE		:= include
LIB		:= lib

LIBRARIES	:= -lavcodec -lavformat -lavdevice -lavutil -lavfilter -lswresample -lswscale -lm
EXECUTABLE	:= pixelsort 


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.c
	$(CXX) $(CXX_FLAGS) $(OPTIMIZATION) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*
