#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <regex.h>

#define TOTAL_TO 20
#define MAXMAILS = 32000
#define MAX_USERNAME 50
#define MAX_PASSWORD 30
#define MAX_SUBJECT 100
#define MAX_CONTENT 2000
#define NUM_OF_CLIENTS 20
#define WELCOME_LENGTH 32
#define DEFAULT_PORT 6423

int main(int argc, char* argv[])
{
	char welcomeMessage[WELCOME_LENGTH], usernameAndPassword[MAX_USERNAME+MAX_PASSWORD+2], password[MAX_PASSWORD+1], connected[2], clientReq[18];
	int sockDes, recvLen, sendLen, reqRet, success;
	regex_t nmclReq;
	success = regcomp(&nmclReq, "(GET_MAIL |DELETE_MAIL )[1,9][0,9]*", 0);

	/* Initialize address struct: */
	struct sockaddr_in serverAddr;
	serverAddr.sin_family=AF_INET;
	if(argc>1) // got ip addr
	{
		success = inet_aton(argv[1], &serverAddr.sin_addr);
		if(success==0)
		{
			//TODO errors
		}
	}else{
		inet_aton("127.0.0.1", &serverAddr.sin_addr);
	}
	if(argc>2) // got port number
	{
		success = sscanf(argv[2], "%hu", &serverAddr.sin_port);
		if(success!=1)
		{
			//TODO errors
		}
		serverAddr.sin_port = htons(serverAddr.sin_port);
	}else{
		serverAddr.sin_port = DEFAULT_PORT;
	}

	/* Socket initialization */
	sockDes = socket(PF_INET, SOCK_STREAM, 0);
	success = connect(sockDes, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(success==-1)
	{
		//TODO errors
	}
	recvLen = recv(sockDes, welcomeMessage, WELCOME_LENGTH, 0);

	/* Authentication */
	printf("%s\n", welcomeMessage);
	printf("User: ");
	fgets(usernameAndPassword, MAX_USERNAME, stdin);
	if(!success)
	{
		//TODO errors
	}
	printf("Password: ");
	fgets(password, MAX_PASSWORD, stdin);
	if(!success)
	{
		//TODO errors
	}
	strcat(usernameAndPassword,"\t");
	strcat(usernameAndPassword,password);
	sendLen = send(sockDes, usernameAndPassword, strlen(usernameAndPassword)+1, 0);
	recvLen = recv(sockDes, connected, 2, 0);
	if(connected[0] != 'Y')
	{
		printf("Could not connect to the server.\nUsername and password combination is incorrect\n");
		return 0;
	}else{
		printf("Connected to server\n");
		do
		{
			fgets(clientReq, 18, stdin);
			if(!success)
			{
				//TODO errors
			}
			if(strcmp(clientReq,"SHOW_INBOX")==0)
			{
			}
			else if(strcmp(clientReq,"COMPOSE")==0)
			{

			}
			else if(!regexec(&nmclReq, clientReq, 0, NULL, 0))
			{

			}
			else if(!strcmp(clientReq, "QUIT"))
			{
				//TODO errors
			}
		}while(strcmp(clientReq, "QUIT"));
	}
	return 0;
}