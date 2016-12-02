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
#define INBOX_SIZE (9+MAX_USERNAME+MAX_SUBJECT)*MAXMAILS
#define MAIL_SIZE 28+(MAX_USERNAME*(TOTAL_TO+1))+MAX_SUBJECT+MAX_CONTENT

int main(int argc, char* argv[])
{
	char welcomeMessage[WELCOME_LENGTH], usernameAndPassword[MAX_USERNAME+MAX_PASSWORD+2], password[MAX_PASSWORD+1], inbox[INBOX_SIZE+1];
	char recps[TOTAL_TO*(MAX_USERNAME+1)], subj[MAX_SUBJECT+1], ctnt[MAX_CONTENT+1], inMail[MAIL_SIZE+1];
	char connected[2], clientReq[18], reqNum[3], nmclReq[8];
	int sockDes, recvLen, sendLen, reqRet, success, numIndex, ar, isGet;
	regex_t nmclReqPattern;
	success = regcomp(&nmclReqPattern, "(GET_MAIL |DELETE_MAIL )[1,9][0,9]*", 0);

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
			if(strcmp(clientReq,"SHOW_INBOX")==0) // Show request
			{
				sendLen = send(sockDes, "1", 2, 0);
				recvLen = recv(sockDes, inbox, INBOX_SIZE+1, 0);
				printf("%s\n", inbox);
			}
			else if(strcmp(clientReq, "QUIT")==0) // Quit request
			{
				sendLen = send(sockDes, "4", 2, 0)
				success = close(sockdes);
				if(success==-1)
				{
					//TODO errors
				}
			}
			else if(strcmp(clientReq,"COMPOSE")==0) // Compose request
			{
				printf("To: ");
				fgets(recps, TOTAL_TO*(MAX_USERNAME+1), stdin);
				printf("Subject: ");
				fgets(subj, MAX_SUBJECT+1, stdin);
				printf("Text: ");
				fgets(ctnt, MAX_CONTENT+1, stdin);
				sendLen = send(sockDes, "5", 2, 0);
				//TODO finish
			}
			else if(!regexec(&nmclReqPattern, clientReq, 0, NULL, 0))
			{
				if(clientReq[0]=='G') // Get request
				{
					reqNum = {'2',' ','\0'};
					numIndex = 9;
					ar=1;
					isGet=1;
				}
				else if(clientReq[0]=='D') // Delete request
				{
					reqNum = {'3',' ','\0'};
					numIndex = 13;
					ar=1;
					isGet=0;
				}
				else
				{
					//TODO errors
					ar=0;
					isGet=0;
				}
				if(ar)
				{
					memset(nmclReq, '\0', 8);
					strcat(nmclReq, reqNum);
					strcat(nmclReq, &clientReq[numIndex]); //TODO check if this work
					sendLen = send(sockDes, nmclReq, 8, 0);
					if(isGet)
					{
						recvLen=recv(sockDes, inMail, MAIL_SIZE, 0);
						printf("%s\n",inMail);
					}
			}
			else
			{
				//TODO errors
			}
		}while(strcmp(clientReq, "QUIT"));
	}
	return 0;
}