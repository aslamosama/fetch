CXX = g++
CXXFLAGS = -std=c++17 -Ofast
LDFLAGS = -lX11
TARGET = fetch
SRC = fetch.cpp
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

install: $(TARGET)
	install -Dm755 $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)
