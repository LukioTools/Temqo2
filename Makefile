CC ?= g++                               # FILL: the compiler
CXX ?= g++                              # FILL: the compiler
CFLAGS := -Wall                 # FILL: compile flags
CXXFLAGS := -Wall               # FILL: compile flags

DEPENDENCIES := -lsfml-audio -lsfml-system # -ltag -lvlc -lfreeimage #-lm -pthread 


# compile macros
TARGET_FILE := vtui.cpp # FILL: target name
OUT_NAME := temqo


# default rule
default: build exec

exec: 
	@echo "Executing: ./$(OUT_NAME)"
	@./$(OUT_NAME)

build:     
	@ $(CXX) $(TARGET_FILE) $(DEPENDENCIES) $(CXXFLAGS) -o $(OUT_NAME)


