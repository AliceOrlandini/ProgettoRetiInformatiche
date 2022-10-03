# make rule primaria con dummy target 'all'
all: dev serv

# make rule per il device 
dev: device.o ./client/src/device_commands.o ./network/src/network.o ./client/src/online_users.o ./client/src/users_chatting_with.o
	gcc -Wall -o dev device.o ./client/src/device_commands.o ./network/src/network.o ./client/src/online_users.o ./client/src/users_chatting_with.o

device.o: device.c
	gcc -c -Wall -o device.o device.c

device_commands.o: device_commands.c 
	gcc -c -Wall -o ./client/src/device_commands.o ./client/src/device_commands.c

online_users.o: online_users.c 
	gcc -c -Wall -o ./client/src/online_users.o ./client/src/online_users.c

users_chatting_with.o: users_chatting_with.c 
	gcc -c -Wall -o ./client/src/users_chatting_with.o ./client/src/users_chatting_with.c

network.o: network.c 
	gcc -c -Wall -o ./network/src/network.o ./network/src/network.c

# make rule per il server 
serv: server.o ./server/src/server_commands.o ./server/src/device_requests.o ./network/src/network.o ./server/src/pending_messages.o
	gcc -Wall -o serv server.o ./server/src/server_commands.o ./server/src/device_requests.o ./network/src/network.o ./server/src/pending_messages.o

server.o: server.c
	gcc -c -Wall -o server.o server.c 

server_commands.o: server_commands.c 
	gcc -c -Wall -o ./server/src/server_commands.o ./server/src/server_commands.c

device_requests.o: device_requests.c 
	gcc -c -Wall -o ./server/src/device_requests.o ./server/src/device_requests.c

pending_messages.o: pending_messages.c 
	gcc -c -Wall -o ./server/src/pending_messages.o ./server/src/pending_messages.c

# pulizia dei file della compilazione (eseguito con make clean)
clean:
	rm *o dev serv