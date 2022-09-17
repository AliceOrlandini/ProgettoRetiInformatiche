# make rule primaria con dummy target 'all'
all: dev serv

# make rule per il device 
dev: device.o ./client/src/device_commands.o ./client/src/network.o
	gcc -Wall -o dev device.o ./client/src/device_commands.o ./client/src/network.o

device.o: device.c
	gcc -c -Wall -o device.o device.c

device_commands.o: device_commands.c 
	gcc -c -Wall -o ./client/src/device_commands.o ./client/src/device_commands.c

network.o: network.c 
	gcc -c -Wall -o ./client/src/network.o ./client/src/network.c

# make rule per il server 
serv: server.o ./server/src/server_commands.o
	gcc -Wall -o serv server.o ./server/src/server_commands.o

server.o: server.c
	gcc -c -Wall -o server.o server.c 

server_commands.o: server_commands.c 
	gcc -c -Wall -o ./server/src/server_commands.o ./server/src/server_commands.c

# pulizia dei file della compilazione (eseguito con make clean)
clean:
	rm *o dev serv