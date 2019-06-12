all:
	gcc server.c -lsqlite3 -o server
	gcc client.c -o client

clean:
	rm server client
