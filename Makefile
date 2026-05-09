CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

SRC = src/main.cpp src/system.cpp

TARGET = monitor

all:
	$(CXX) $(CXXFLAGS) $(SRC) -Iinclude -o $(TARGET)

clean:
	rm -f $(TARGET)