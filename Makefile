# Makefile para clienteftp

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
LDFLAGS = -lpthread

SOURCES = clienteftp.c \
          errexit.c \
          connectTCP.c \
          passiveTCP.c \
          passivesock.c \
          connectsock.c

TARGET  = clienteftp

# Regla por defecto
all: $(TARGET)

# Compilar igual que tu comando de gcc (sin usar .o intermedios)
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

# Ejecutar el cliente
run: $(TARGET)
	./$(TARGET)

# Limpiar ejecutable
clean:
	rm -f $(TARGET)

