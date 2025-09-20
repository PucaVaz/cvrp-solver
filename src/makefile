CPP = g++
CPPFLAGS = -std=c++20 -O3 -Wall

SRCDIR = .
OBJDIR = ../obj
BINDIR = ../bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/apa_jpbike

# Cria diretórios obj/ e bin/
$(shell mkdir -p $(OBJDIR) $(BINDIR))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CPP) $(OBJECTS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o $(TARGET)

test: $(TARGET)
	$(TARGET) --instance ../instances/instancias_teste/n14_q30.txt

run: $(TARGET)
	$(TARGET) --instance ../instances/instancias_teste/n14_q30.txt --out ../outputs/

.PHONY: all clean test run
