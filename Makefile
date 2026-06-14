CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++11
LDFLAGS =

SRCS = src/main.cpp src/Instance.cpp src/Solution.cpp src/GeneticAlgorithm.cpp
OBJS = $(SRCS:.cpp=.o)

# Nombre del ejecutable segun plataforma: en Windows (MSYS2/MinGW) g++ genera un
# .exe, por lo que el target debe llamarse igual para que make no relinkee de mas
# y para que 'make clean' lo elimine correctamente. En Linux queda sin extension.
ifeq ($(OS),Windows_NT)
    TARGET = evcs_solver.exe
else
    TARGET = evcs_solver
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
