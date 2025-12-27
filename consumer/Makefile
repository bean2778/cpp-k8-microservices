CXX = g++
CXXFLAGS = -std=c++17 -I.. -Wall -pthread
TARGET = consumer
SRC = consumer.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)