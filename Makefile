# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -fPIC -IInclude
LDFLAGS = -LBuild/lib -Wl,-rpath,Build/lib
LDLIBS = -lutils

# Project Structure
SRC_DIR = Source
INC_DIR = Include
OBJ_DIR = Build/obj
LIB_DIR = Build/lib
BIN_DIR = Build/bin

# Targets
TARGET = $(BIN_DIR)/hyde-tui
LIB_NAME = utils
LIB_SO = $(LIB_DIR)/lib$(LIB_NAME).so

# Sources and Objects
MAIN_SRC = $(SRC_DIR)/main.cpp
MAIN_OBJ = $(OBJ_DIR)/$(SRC_DIR)/main.o

LIB_SRCS = $(SRC_DIR)/utils.cpp
LIB_OBJS = $(OBJ_DIR)/$(SRC_DIR)/utils.o

# Phony Targets
.PHONY: all clean install uninstall

# Default
all: $(TARGET)

# Main Binary
$(TARGET): $(MAIN_OBJ) $(LIB_SO)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(MAIN_OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

# Shared Library
$(LIB_SO): $(LIB_OBJS)
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared -o $@ $^

# Generic Compile Rule
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run executable
run: 
	$(TARGET)

# Clean
clean:
	rm -rf Build

# Install
install: $(TARGET) $(LIB_SO)
	install -Dm755 $(TARGET) ~/.local/bin/hyde-tui
	install -Dm755 $(LIB_SO) ~/.local/lib/libutils.so
	install -Dm644 $(INC_DIR)/utils.hpp ~/.local/include/utils.hpp

# Uninstall
uninstall:
	rm -f ~/.local/bin/hyde-tui
	rm -f ~/.local/lib/libutils.so
	rm -f ~/.local/include/utils.hpp
