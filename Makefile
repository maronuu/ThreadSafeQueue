# PROGNAME := main queueTest
INCDIR := include
SRCDIR := src
LIBDIR := lib
OUTDIR := build
TARGET := $(OUTDIR)/main $(OUTDIR)/queueTest
SRCS := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/$(LIBDIR)/*.c)
OBJS := $(addprefix $(OUTDIR)/,$(patsubst %.c,%.o,$(SRCS)))
#$(warning $(OBJS))

CC = gcc
CFLAGS = -Wall -O2 -pthread -I $(INCDIR)

# .PHONY: all clean
all: $(TARGET)

$(OUTDIR)/main: $(SRCDIR)/main.c $(SRCDIR)/$(LIBDIR)/safeQueue.c
	$(CC) $(CFLAGS) $^ -lm -o $@

$(OUTDIR)/queueTest: $(SRCDIR)/queueTest.c $(SRCDIR)/$(LIBDIR)/safeQueue.c
	$(CC) $(CFLAGS) $^ -lm -o $@

$(OUTDIR)/%.o:%.c
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CC) $(CFLAGS) -o $@ -c $< -lm

clean:
	rm -rf $(OUTDIR)