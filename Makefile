CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

SRC = src/main.cpp src/server.cpp src/request.cpp src/response.cpp src/router.cpp
TARGET = server

TEST_SRC = tests/test_request.cpp src/request.cpp
TEST_TARGET = test_request

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SRC)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all clean test