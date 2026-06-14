CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++11
LDFLAGS =

SRCDIR   = src
BUILDDIR = build

# Fuentes en src/; los objetos .o (y los .d de dependencias) se generan en build/
# para no ensuciar src/. wildcard toma todos los .cpp: agregar un modulo nuevo no
# requiere editar este Makefile.
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

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

# -MMD -MP genera, junto a cada .o, un .d con las dependencias de cabeceras, para
# que make recompile solo cuando cambia un .h. La carpeta build/ se crea sola.
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

# Incluye las dependencias generadas (si existen); el guion evita error la 1a vez.
-include $(DEPS)

.PHONY: all clean
