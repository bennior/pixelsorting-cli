CXX		:= gcc
CXX_FLAGS 	:= -Wall -Wextra -ggdb
OPT		:= -O3 -ffast-math


BIN		:= bin
SRC		:= src
INCLUDE		:= include
LIB		:= lib

LIBRARIES	:= -lavformat -lavcodec -lswscale -lavutil -lm
EXECUTABLE	:= pixelsort 

ifeq ($(INSTALL_PREFIX),)
	INSTALL_PREFIX	:= /usr/local
endif

UNINSTALL	:= $(shell ls $(LIB))

all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE) --help

$(BIN)/$(EXECUTABLE): $(SRC)/*.c
	$(CXX) $(CXX_FLAGS) $(OPT) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)
	@printf "\nYou can run now 'make install'\n"

clean:
	-rm $(BIN)/*

install: $(BIN)/$(EXECUTABLE)
	install -Dt $(INSTALL_PREFIX)/bin/ $(BIN)/$(EXECUTABLE)
	install -Dt $(INSTALL_PREFIX)/lib/ $(LIB)/*.so*

uninstall: 
	@cd $(INSTALL_PREFIX)/lib; rm $(UNINSTALL)
	rm $(INSTALL_PREFIX)/bin/$(EXECUTABLE)
