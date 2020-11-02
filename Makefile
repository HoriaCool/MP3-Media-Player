# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# libraries needed for the project:
AUDIO_FLAGS = -lmpg123 -lao
PTHREAD_FLAGS = -lpthread -lrt
GTK_FLAGS = `pkg-config --cflags --libs gtk+-3.0`

# compiler flags:
CFLAGS = $(GTK_FLAGS) $(AUDIO_FLAGS) $(PTHREAD_FLAGS)

# the build target executable:
TARGET = mp3.exe

# the source code, entry point:
SOURCE = ./src/main.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(SOURCE) $(CFLAGS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
