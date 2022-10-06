SERVER_SOURCE = pam_demo_server.c
CLIENT_SOURCE = pam_demo_client.c
all:
	gcc $(SERVER_SOURCE) -fPIC -shared -o pam_demo_server.so
	gcc $(CLIENT_SOURCE) -lpam -o pam_demo_client
clean:
	 rm pam_demo_server.so 
	 rm pam_demo_client 
