# Makefile для сервера векторных вычислений
# Автор: Судариков А.В.

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -I./src
LDFLAGS = -lssl -lcrypto
TARGET = server
SRCDIR = src
SOURCES = $(SRCDIR)/main.cpp \
          $(SRCDIR)/Server.cpp \
          $(SRCDIR)/Config.cpp \
          $(SRCDIR)/Database.cpp \
          $(SRCDIR)/Logger.cpp \
          $(SRCDIR)/Authenticator.cpp \
          $(SRCDIR)/VectorProcessor.cpp
HEADERS = $(SRCDIR)/Server.h \
          $(SRCDIR)/Config.h \
          $(SRCDIR)/Database.h \
          $(SRCDIR)/Logger.h \
          $(SRCDIR)/Authenticator.h \
          $(SRCDIR)/VectorProcessor.h
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

install:
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "Сервер установлен в /usr/local/bin/$(TARGET)"

uninstall:
	rm -f /usr/local/bin/$(TARGET)

run: $(TARGET)
	./$(TARGET) -c test_db.txt -l server.log -p 33333

debug: $(TARGET)
	./$(TARGET) -c test_db.txt -l server.log -p 33333

.PHONY: all clean install uninstall run debug
