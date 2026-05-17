CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

SRC = src/main.cpp \
      src/system.cpp \
      src/display.cpp \
      src/arguments.cpp \
      src/history.cpp \
      src/gpu.cpp

INC = -Iinclude

TARGET = monitor

all:
	$(CXX) $(CXXFLAGS) $(SRC) $(INC) -o $(TARGET)

clean:
	rm -f $(TARGET)