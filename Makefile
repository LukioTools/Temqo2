CC ?= g++                               # FILL: the compiler
CXX ?= g++                              # FILL: the compiler
CFLAGS := -Wall                 # FILL: compile flags
CXXFLAGS := -Wall               # FILL: compile flags

DEPENDENCIES := 


# compile macros
TARGET_FILE := mouse.cpp # FILL: target name
OUT_NAME := mouse


# default rule
default: build exec



exec: 
	./$(OUT_NAME)

build:     
	@ $(CXX) $(TARGET_FILE) $(DEPENDENCIES) $(CXXFLAGS) -o $(OUT_NAME)


