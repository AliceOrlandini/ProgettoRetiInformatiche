# make rule primaria con dummy target 'all'
all: dev serv

# make rule per il device 
dev: device.o ./client/device_commands.o ./client/network.o
	gcc -Wall -o dev device.o ./client/device_commands.o ./client/network.o

device.o: device.c
	gcc -c -Wall -o device.o device.c

device_commands.o: device_commands.c 
	gcc -c -Wall -o ./client/device_commands.o ./client/device_commands.c

network.o: network.c 
	gcc -c -Wall -o ./client/network.o ./client/network.c

# make rule per il server 
serv: server.o ./server/server_commands.o
	gcc -Wall -o serv server.o ./server/server_commands.o

server.o: server.c
	gcc -c -Wall -o server.o server.c 

server_commands.o: server_commands.c 
	gcc -c -Wall -o ./server/server_commands.o ./server/server_commands.c

# pulizia dei file della compilazione (eseguito con make clean)
clean:
	rm *o dev serv