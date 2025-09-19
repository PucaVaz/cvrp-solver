CPP = g++
CPPFLAGS = -std=c++20 -O3 -Wall

SRCDIR = .
OBJDIR = ../obj

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = ../jp_bike_solver

# create obj directory
$(shell mkdir -p $(OBJDIR))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CPP) $(OBJECTS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o $(TARGET)

test: $(TARGET)
	$(TARGET) ../instances/instancias_teste/n12_q20.txt

.PHONY: all clean test
