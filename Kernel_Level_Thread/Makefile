# HEADERS = program.h headers.h

default: kt_test

# program.o: program.c $(HEADERS)
#     gcc -c program.c -o program.o

functions.o: functions.c
	gcc -c -std=gnu99 -pthread functions.c -o functions.o -lrt

kt_test: kt_test.c functions.o
	gcc -g -std=gnu99 -pthread kt_test.c functions.o -o kt_test -lrt

clean:
	-rm -f kt_test functions.o

.PHONY: clean

