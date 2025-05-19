CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -fPIC
LDFLAGS = -lncurses -ldl -ljsoncpp

SCRIPT_DIR = Scripts
OBJ_DIR = Build/obj
LIB_DIR = Build/lib

TARGET = hyde-tui

SRCS = HyDE_TUI.cpp
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

SCRIPT_SRCS = $(SCRIPT_DIR)/system_patcher.cpp $(SCRIPT_DIR)/fresh_install.cpp
SCRIPT_OBJS = $(patsubst $(SCRIPT_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SCRIPT_SRCS))
SCRIPT_SOS = $(patsubst $(SCRIPT_DIR)/%.cpp,$(LIB_DIR)/%.so,$(SCRIPT_SRCS))

.PHONY: all clean install uninstall

all: $(TARGET) $(SCRIPT_SOS)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(LIB_DIR)/%.so: $(OBJ_DIR)/%.o
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared -o $@ $< $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SCRIPT_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(TARGET)

install: $(TARGET) $(SCRIPT_SOS)
	install -Dm755 $(TARGET) ~/.local/bin/$(TARGET)
	install -Dm755 $(SCRIPT_SOS) ~/.local/lib/hyde-tui/

uninstall:
	rm -f ~/.local/bin/$(TARGET)
	rm -f ~/.local/lib/hyde-tui/*.so
