#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char* argv[])
{
	const int TOTAL_TO = 20, MAXMAILS = 32000, MAX_USERNAME = 50, MAX_PASSWORD = 30, MAX_SUBJECT = 100, MAX_CONTENT = 2000, NUM_OF_CLIENTS = 20;
	const int WELCOME_LENGTH = 33;
	const short DEFAULT_PORT = 6423;
	char welcomeMessage[WELCOME_LENGTH], username[MAX_USERNAME], password[MAX_PASSWORD];
	int recLen;

	/* Initialize address struct: */
	struct sockaddr_in serverAddr;
	serverAddr.sin_family=AF_INET;
	int success;
	if(argc>1) // got ip addr
	{
		success = inet_aton(argv[1], &serverAddr.sin_addr);
		if(success==0) //Invalid address format
		{
			//TODO errors
		}
	}else{
		inet_aton("127.0.0.1", &serverAddr.sin_addr)
	}
	if(argc>2) // got port number
	{
		success = sscanf(argv[2], "%hu", &serverAddr.sin_port);
		if(success!=1) // Invalid port
		{
			//TODO errors
		}
		*serverAddr.sin_port = htons(*serverAddr.sin_port);
	}else{
		serverAddr.sin_port = DEFAULT_PORT;
	}
	int sockDes = socket(PF_INET, SOCK_STREAM, 0);
	success = connect(sockDes, serverAddr, sizeof(serverAddr));
	if(success==-1)// error
	{
		//TODO errors
	}
	recLen = recv(sockDes, welcomeMessage, WELCOME_LENGTH);
	printf("%s\n", *welcomeMessage);
	printf("User: ");
	username = scanf("%s");
}