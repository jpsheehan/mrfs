CC=gcc
CFLAGS=-Wall -g -Og --std=gnu18

all: mrfs test

test: MrbsBookingTest MrbsTest MrfsTest Base64Test Base64FileTest MrfsDirectoryTest

clean:
	rm -f *.o ./main ./MrfsDirectoryTest ./MrbsTest ./Base64Test ./MrfsTest ./Base64FileTest ./vgcore.* ./mrfs ./MrbsBookingTest
	fusermount -u foo

rebuild:
	make clean
	make all

format:
	astyle -n -A3 -t4 *.c *.h

mount:
	./mrfs ./foo 1902 12 21 18 30 215

unmount:
	fusermount -u ./foo

#################################################
## OUTPUT FILES

mrfs: src/MrfsFuse.c LinkedList.o KeyValueList.o Base64.o Mrbs.o Http.o Mrfs.o
	$(CC) $(CFLAGS) `pkg-config fuse --cflags` `curl-config --cflags` $^ -o $@ `curl-config --libs` `pkg-config fuse --libs`

#################################################
## TEST FILES

MrfsDirectoryTest: src/MrfsDirectoryTest.c Mrfs.o Base64.o Mrbs.o KeyValueList.o LinkedList.o Http.o
	$(CC) $(CFLAGS) `curl-config --cflags` $^ -o $@ `curl-config --libs`

MrfsTest: src/MrfsTest.c Mrfs.o Base64.o Mrbs.o KeyValueList.o LinkedList.o Http.o
	$(CC) $(CFLAGS) `pkg-config fuse --libs --cflags` `curl-config --cflags` $^ -o $@ `curl-config --libs`

MrbsTest: src/MrbsTest.c LinkedList.o KeyValueList.o Base64.o Mrbs.o Http.o
	$(CC) $(CFLAGS) `curl-config --cflags` $^ -o $@ `curl-config --libs`

MrbsBookingTest: src/MrbsBookingTest.c LinkedList.o KeyValueList.o Base64.o Mrbs.o Mrfs.o Http.o
	$(CC) $(CFLAGS) `curl-config --cflags` $^ -o $@ `curl-config --libs`

Base64FileTest: src/Base64FileTest.c Base64.o
	$(CC) $(CFLAGS) $^ -o $@

Base64Test: src/Base64Test.c Base64.o
	$(CC) $(CFLAGS) $^ -o $@

HttpFileTest: src/HttpFileTest.c LinkedList.o KeyValueList.o Base64.o Http.o 
	$(CC) $(CFLAGS) $^ -o $@

#################################################
## LIBRARY FILES

Base64.o: src/Base64.c
	$(CC) $(CFLAGS) -c $^ -o $@

Http.o: src/Http.c 
	$(CC) $(CFLAGS) -c $^ -o $@

Mrfs.o: src/Mrfs.c
	$(CC) $(CFLAGS) -c $^ -o $@

Mrbs.o: src/Mrbs.c
	$(CC) $(CFLAGS) -c $^ -o $@

KeyValueList.o: src/KeyValueList.c 
	$(CC) $(CFLAGS) -c $^ -o $@
	
LinkedList.o: src/LinkedList.c
	$(CC) $(CFLAGS) -c $^ -o $@
	

.PHONY: all clean format rebuild server test mount unmount

