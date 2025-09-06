CPP = g++
CPPFLAGS = -std=c++20 -O3 -Wall

SRCDIR = .
OBJDIR = ../obj

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = ../jp_bike_solver.o

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
	$(TARGET) ../instances/instancia_pdf.txt

.PHONY: all clean test
