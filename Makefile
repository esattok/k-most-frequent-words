all: proctopk threadtopk

proctopk: proctopk.c
	gcc -Wall -g -o proctopk proctopk.c -lrt

threadtopk: threadtopk.c
	gcc -Wall -g -o threadtopk threadtopk.c -lpthread -lrt

clean: 
	rm -fr proctopk threadtopk *~ *.o