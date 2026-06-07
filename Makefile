CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++11
LDFLAGS =

SRCS = src/main.cpp src/Instance.cpp src/Solution.cpp src/GeneticAlgorithm.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = evcs_solver

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
