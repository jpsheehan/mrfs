CC=gcc
CFLAGS=-Wall -g -Og --std=gnu18

all: main mrfs test

test: MrbsBookingTest MrbsTest MrfsTest HttpTest Base64Test Base64FileTest HttpFileTest MrfsDirectoryTest

clean:
	rm -f *.o ./main ./HttpTest ./MrfsDirectoryTest ./MrbsTest ./Base64Test ./MrfsTest ./Base64FileTest ./HttpFileTest ./vgcore.* ./mrfs ./MrbsBookingTest
	fusermount -u foo

rebuild:
	make clean
	make all

format:
	astyle -n -A3 -t4 *.c *.h

mount:
	valgrind ./mrfs ./foo 1901 12 14 15 30 226

unmount:
	fusermount -u ./foo

#################################################
## OUTPUT FILES

main: src/main.c Http.o Mrbs.o Mrfs.o Base64.o KeyValueList.o LinkedList.o
	$(CC) $(CFLAGS) $^ -o $@

mrfs: src/MrfsFuse.c LinkedList.o KeyValueList.o Base64.o Http.o Mrbs.o Mrfs.o
	$(CC) $(CFLAGS) `pkg-config fuse --libs --cflags` $^ -o $@

#################################################
## TEST FILES

HttpTest: src/HttpTest.c LinkedList.o KeyValueList.o Base64.o Http.o
	$(CC) $(CFLAGS) $^ -o $@

MrfsDirectoryTest: src/MrfsDirectoryTest.c Mrfs.o Base64.o Mrbs.o Http.o KeyValueList.o LinkedList.o
	$(CC) $(CFLAGS) $^ -o $@

MrfsTest: src/MrfsTest.c Mrfs.o Base64.o Mrbs.o Http.o KeyValueList.o LinkedList.o
	$(CC) $(CFLAGS) `pkg-config fuse --libs --cflags` $^ -o $@

MrbsTest: src/MrbsTest.c LinkedList.o KeyValueList.o Base64.o Http.o Mrbs.o
	$(CC) $(CFLAGS) $^ -o $@

MrbsBookingTest: src/MrbsBookingTest.c LinkedList.o KeyValueList.o Base64.o Http.o Mrbs.o Mrfs.o
	$(CC) $(CFLAGS) $^ -o $@

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

