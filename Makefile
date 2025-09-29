INCS = -I./include
LDFLAGS = -L. -lshmcomm -lpthread -lrt -lelf
CC = gcc
PROGS = questbinloader 

.PHONY: all clean

all: $(PROGS)

$(PROGS): %: %.o questbinloader_helper.o
	$(CC) $(INCS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(INCS) -D_GNU_SOURCE -c -o $@ $<

clean:
	rm -rf $(PROGS) *.o
