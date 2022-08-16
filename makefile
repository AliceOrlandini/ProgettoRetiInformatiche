# make rule primaria con dummy target 'all'
all: dev serv

# make rule per il device 
dev: device.o
	gcc -Wall -o dev device.o

device.o: device.c
	gcc -c -Wall -o device.o device.c

# make rule per il server 
serv: server.o ./server/commands.o
	gcc -Wall -o serv server.o ./server/commands.o

server.o: server.c
	gcc -c -Wall -o server.o server.c 

commands.o: commands.c 
	gcc -c -Wall -o ./server/commands.o ./server/commands.c

# pulizia dei file della compilazione (eseguito con make clean)
clean:
	rm *o dev serv