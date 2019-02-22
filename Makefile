# Makefile

CC = gcc
CFLAGS = -std=c11 -pedantic -Wall

SRCDIR = src
OBJDIR = obj
BINDIR = bin

DEBUG = -g

.PHONY: all
all : $(BINDIR)/swordx
	@echo Created swordx executable in /bin.

$(BINDIR)/swordx: $(OBJDIR)/swordx.o $(OBJDIR)/avltree.o $(OBJDIR)/trie.o $(OBJDIR)/list.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/swordx.o: $(SRCDIR)/swordx.c $(OBJDIR)/avltree.o $(OBJDIR)/trie.o $(OBJDIR)/list.o
	$(CC) $(CFLAGS) -c -o $@ $<

avltree: $(OBJDIR)/avltree.o

$(OBJDIR)/avltree.o: $(SRCDIR)/lib/avltree/avltree.c $(OBJDIR)/trie.o
	$(CC) $(CFLAGS) -c -o $@ $<

trie: $(OBJDIR)/trie.o

$(OBJDIR)/trie.o: $(SRCDIR)/lib/trie/trie.c $(OBJDIR)/list.o
	$(CC) $(CFLAGS) -c -o $@ $<

list: $(OBJDIR)/list.o

$(OBJDIR)/list.o: $(SRCDIR)/lib/list/list.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	-rm $(BINDIR)/swordx $(OBJDIR)/*.o

.PHONY: install
install:
	@cp $(BINDIR)/swordx /usr/local/bin/swordx
	@echo "Installation completed." 

.PHONY: uninstall
uninstall:
	@rm /usr/local/bin/swordx
	@echo "Uninstallation completed."
