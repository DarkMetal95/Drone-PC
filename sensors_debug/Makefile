CC=gcc

BDIR=build
INCDIR=include
SRCDIR=src

CFLAGS=-Wall -I$(INCDIR)
LDFLAGS=-lbluetooth -lncurses -lm -lGL -lGLU -lglut

EXEC=sensors_debug

all: setup $(EXEC)

$(EXEC): $(BDIR)/main.o
	$(CC) -o $@ $^ $(LDFLAGS)
$(BDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)
setup:
	mkdir -p build
clean:
	rm -rf build
mrproper: clean
	rm -f $(EXEC)
