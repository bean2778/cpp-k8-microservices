CXX = g++
CXXFLAGS = -std=c++17 -Wall -I.. -pthread
TARGET = producer
SRC = producer.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)