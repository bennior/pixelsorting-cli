CXX		:= g++
CXX_FLAGS 	:= -Wall -Wextra -std=c++17 -ggdb
OPTIMIZATION	:= -O1

BIN		:= bin
SRC		:= src
INCLUDE		:= include
LIB		:= lib

LIBRARIES	:= -lavcodec -lavformat -lavdevice -lavutil -lavfilter -lswresample -lswscale 
EXECUTABLE	:= pixelsort 


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) $(OPTIMIZATION) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*
