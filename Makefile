CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

SRC = src/main.cpp src/server.cpp src/request.cpp
TARGET = server

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean