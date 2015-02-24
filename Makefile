CC=gcc

BDIR=build
INCDIR=include
SRCDIR=src

CFLAGS=-Wall -I$(INCDIR)
LDFLAGS=-lbluetooth -lncurses -lm -lGL -lGLU -lglut

EXEC1=sensors_debug
EXEC2=sensors_3D_debug

all: setup $(EXEC1) $(EXEC2)

$(EXEC1): $(BDIR)/main.o
	$(CC) -o $@ $^ $(LDFLAGS)
$(EXEC2): $(BDIR)/main_debug.o
	$(CC) -o $@ $^ $(LDFLAGS)
$(BDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)
setup:
	mkdir -p build
clean:
	rm -rf build
mrproper: clean
	rm -f $(EXEC)
