/* TomeNET world server test code copyright 2002 Richard Smith. */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <errno.h>

#define PORT 18360
#define ADDR INADDR_ANY

struct sockaddr_in sa;

int main(int argc, char *argv[]){
	int ser;
	ser=createsocket(PORT, ADDR);
	if(ser!=-1){
		world(ser);
		close(ser);
	}
}

int createsocket(int port, unsigned long ip){
	int option=1;
	struct sockaddr_in s_in;
	int check;
	int ss;

	/* Create a socket */
	ss=socket(AF_INET, SOCK_STREAM, 0);
	if(ss < 0) return(-1);

	/* Set option on socket not to wait on shutdown */
	setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, (void*)&option, sizeof(int));

	s_in.sin_family=AF_INET;
	s_in.sin_addr.s_addr=ip;
	s_in.sin_port=htons(port);

	check=bind(ss, (struct sockaddr *)&s_in, sizeof(struct sockaddr_in));
	if(check != -1){
		check=listen(ss, 5);
		if(check != -1) return(ss);
	}
	close(ss);
	return(-1);
}
