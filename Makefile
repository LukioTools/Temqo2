CC ?= g++                               # FILL: the compiler
CXX ?= g++                              # FILL: the compiler
CFLAGS := -Wall                 # FILL: compile flags
CXXFLAGS := -Wall               # FILL: compile flags

DEPENDENCIES := 


# compile macros
TARGET_FILE := main.cpp # FILL: target name
OUT_NAME := temqo


# default rule
default: makedir build exec



.PHONY: exec
exec:
    @$(OUT_NAME)

.PHONY: build
build: 
    @ $(CXX) $(TARGET_FILE) $(DEPENDENCIES) $(CXXFLAGS) -o $(OUT_NAME)


