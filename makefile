main: hw11.c server.h client.h data_t.h
	gcc -g -o hw11 hw11.c server.h client.h data_t.h -pthread -lrt

clean:
	rm hw11

tar:
	tar -cf hw11.tar hw11.c server.h client.h data_t.h makefile
