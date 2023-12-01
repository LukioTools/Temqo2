CC ?= g++                               # FILL: the compiler
CXX ?= g++                              # FILL: the compiler
CFLAGS := -Wall                 		# FILL: compile flags
CXXFLAGS := -Wall    				# FILL: compile flags

DEPENDENCIES := -Wall -ltag -lsdbus-c++ -lfreeimage -lsfml-audio -lsfml-system # -ltag -lvlc -lfreeimage #-lm -pthread 
LINKING := -L./bin/SFML-2.6.1/lib -Wl,-rpath,./bin/SFML-2.6.1/lib 
INCLUDING := -I./bin/SFML-2.6.1/include

# compile macros
TARGET_FILE := temqo.cpp # FILL: target name
OUT_NAME := temqo

DEBUG_ARGS :=  -d /dev/pts/4

# default rule
default: build exec

exec_debug: 
	@echo "Executing: ./$(OUT_NAME)"
	@./$(OUT_NAME) $(DEBUG_ARGS)

exec: 
	./$(OUT_NAME)

build:     
	$(CXX) $(TARGET_FILE) $(INCLUDING) $(LINKING) $(DEPENDENCIES) -o $(OUT_NAME)

d:
	@echo "THIS IS A DBUS TESTING STAGE... BE WARNED\n"
	$(CXX) dbus/dbus.cpp -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -ldbus-1   -o dbus/dbus
	./dbus/dbus

