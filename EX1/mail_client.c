/*Includes*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <stdlib.h>
/*Constants*/
#define TOTAL_TO 20
#define MAXMAILS 32000
#define MAX_USERNAME 50
#define MAX_PASSWORD 30
#define MAX_SUBJECT 100
#define MAX_CONTENT 2000
#define NUM_OF_CLIENTS 20
#define WELCOME_LENGTH 32
#define DEFAULT_PORT 6423
#define INBOX_SIZE (9+MAX_USERNAME+MAX_SUBJECT)*MAXMAILS
#define MAIL_SIZE 28+(MAX_USERNAME*(TOTAL_TO+1))+MAX_SUBJECT+MAX_CONTENT
/*Macros and other general functions*/
void handleError(){printf("an error occured\n");}
#define sendMgetOK(message) sendRet = sendall(sockDes, (message), strlen(message)+1); validate(sendRet); recvRet = recvall(sockDes, ok); validate(recvRet); //TODO handle? x2
#define validate(var) if((var)==-1){handleError();}

/* Slightly modified code from recitation 2's slides */
int sendall(int sd, char* buf, int len)
{
	printf("client: trying to send %s\n", buf);
	int total = 0;
	int n=-1, m=0, bytesleft=len;

	char strnum[11], retnum[11]; // 10 characters (+null terminator) is enough to hold int32's max value. It SHOULD be safe enough...
	memset(strnum, 0, 11);
	memset(retnum, 0, 11);
	sprintf(strnum, "%d", len);
	m=send(sd, strnum, 11, 0)-11;
	printf("client: sent length of %s\n", strnum);
	validate(-(!(!m))) else{
		m=recv(sd, retnum, 11, 0);
		validate(m) else{
			validate(-(!(!strcmp(retnum, strnum)))) else{
				while(total<len)
				{
					printf("client: sending %s\n", buf+total);
					n=send(sd,buf+total, bytesleft, 0);
					validate(n) else{
						total += n;
						bytesleft -= n;
					}
				}
			}
		}
	}
	return n==-1?-1:0;
}
/* not taken from recitation's 2's slides animore */
int recvall(int sd, char* buf)
{
	char strnum[11];
	memset(strnum, 0, 11);
	int m, m2, n=-1, len, total = 0;
	m = recv(sd, strnum, 11, 0)-11;
	printf("client: got %d characters\n", m+11);
	validate(m) else{
		printf("client: got length: %s\n", strnum);
		len = atoi(strnum);
		m2=send(sd, strnum, m+11, 0)-(m+11);
		printf("client: sent %d characters\n", m+11);
		validate(-(!(!m2))) else{
			printf("client: getting data\n");
			while(total<len)
			{
				printf("client: recieving... got %d out of %d\n", total, len);
				n=recv(sd, buf+total, len-total, 0);
				validate(n) else{
					total += n;
				}
			}
		}
	}
	printf("client: recived %s\n", buf);
	return n==-1?-1:0;
}
int readField(const char field[])
{
	int index, r = 0, len=strlen(field);
	for(index = 0; index < len; index++)
	{
		if(getchar()!=field[index])
		{
			r = -1;
		}
	}
	return r;
}
void readInto(char buff[], int size)
{
	char* r = fgets(buff, size, stdin);
	validate(-(!r))else{
		int len = strlen(buff);
		if (*buff && buff[len-1] == '\n') 
		    buff[len-1] = '\0';
	}
}
int main(int argc, char* argv[])
{
	printf("client: starting\n");
	char welcomeMessage[WELCOME_LENGTH], username[MAX_USERNAME+1], password[MAX_PASSWORD+1], inbox[INBOX_SIZE+1];
	char recps[TOTAL_TO*(MAX_USERNAME+1)], subj[MAX_SUBJECT+1], ctnt[MAX_CONTENT+1], inMail[MAIL_SIZE+1];
	char connected[2], clientReq[18], reqNum[3], nmclReq[8], ok[3];
	int sockDes, recvRet, sendRet, success, numIndex, ar, isGet, portNum;
	regex_t nmclReqPattern;
	success = regcomp(&nmclReqPattern, "(GET_MAIL |DELETE_MAIL )[1,9][0,9]*", 0);
	validate(-(!(!success)));
	/* Initialize address struct: */
	printf("client: initializing address struct\n");
	struct sockaddr_in serverAddr;
	serverAddr.sin_family=AF_INET;
	if(argc>1) // got ip addr
	{
		success = inet_pton(AF_INET, argv[1], &serverAddr.sin_addr)-1;
		validate(-(!(!(success-1))));
	}else{
		inet_aton("127.0.0.1", &serverAddr.sin_addr);
	}
	if(argc>2) // got port number
	{
		portNum = htons(atoi(argv[2]));
	}else{
		portNum = htons(DEFAULT_PORT);
	}
	serverAddr.sin_port = portNum;
	/* Socket initialization */
	printf("client: initializing socket\n");
	sockDes = socket(PF_INET, SOCK_STREAM, 0);
	validate(sockDes);

	getsockname(sockDes, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	printf("client: port number: %d, %d, %d\n", ntohs(serverAddr.sin_port), serverAddr.sin_port, portNum);

	printf("client: made socket\n");
	success = connect(sockDes, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	printf("client: connected\n");
	validate(success);
	printf("client: validated connection\n");
	recvRet = recvall(sockDes, welcomeMessage);
	printf("client: recved welcome message\n");
	validate(recvRet); //TODO handle?
	printf("%s\n", welcomeMessage);
	/* Authentication */
	printf("client: user\n");
	readField("User: ");
	readInto(username, MAX_USERNAME);
	printf("client: password\n");
	readField("Password: ");
	readInto(password, MAX_PASSWORD);
	printf("client: sending username and password\n");
	sendMgetOK(username);
	sendRet = sendall(sockDes, password, strlen(password)+1); //TODO handle?
	printf("client: sent username and password\n");
	validate(sendRet); //TODO handle?
	printf("client: validated sent username and password\n");
	recvRet = recvall(sockDes, connected);
	validate(recvRet); //TODO handle?
	printf("client: validated recved data\n");
	if(connected[0] != 'Y')
	{
		printf("Could not connect to the server.\nUsername and password combination is incorrect\n");
		success = close(sockDes);
		validate(success);
		return 0;
	}else{
		printf("client: Connected to server\n");
		do
		{
			readInto(clientReq, 18);
			if(strcmp(clientReq,"SHOW_INBOX")==0) // Show request
			{
				sendRet = sendall(sockDes, "1", 2);
				validate(sendRet); //TODO handle?
				recvRet = recvall(sockDes, inbox);
				validate(recvRet); //TODO handle?
				printf("%s\n", inbox);
			}
			else if(strcmp(clientReq, "QUIT")==0) // Quit request
			{
				printf("client: quitting...\n");
				sendRet = sendall(sockDes, "4", 2);
				validate(sendRet); //TODO handle?
			}
			else if(strcmp(clientReq,"COMPOSE")==0) // Compose request
			{
				success = readField("To: ");
				validate(success);
				readInto(recps, TOTAL_TO*(MAX_USERNAME+1));
				success = readField("Subject: ");
				validate(success);
				readInto(subj, MAX_SUBJECT+1);
				success = readField("Text: ");
				validate(success);
				readInto(ctnt, MAX_CONTENT+1);
				memset(ok, '\0', 3);
				sendMgetOK("5");
				sendMgetOK(recps);
				sendMgetOK(subj);
				sendMgetOK(ctnt);
			}
			else if(!regexec(&nmclReqPattern, clientReq, 0, NULL, 0))
			{
				if(clientReq[0]=='G') // Get request
				{
					reqNum[0]='2'; reqNum[1]=' ';reqNum[2]='\0';
					numIndex = 9;
					ar=1;
					isGet=1;
				}
				else if(clientReq[0]=='D') // Delete request
				{
					reqNum[0]='3'; reqNum[1]=' ';reqNum[2]='\0';
					numIndex = 13;
					ar=1;
					isGet=0;
				}
				else
				{
					handleError();
					ar=0;
					isGet=0;
				}
				if(ar)
				{
					memset(nmclReq, '\0', 8);
					strcat(nmclReq, reqNum);
					strcat(nmclReq, &clientReq[numIndex]); //TODO check if this work
					sendRet = sendall(sockDes, nmclReq, 8);
					validate(sendRet); //TODO handle?
					recvRet = recvall(sockDes, inMail);
					validate(recvRet); //TODO handle?
					if(isGet)
					{
						printf("%s\n",inMail);
					}
				}
			}
			else
			{
				handleError();
			}
		}while(strcmp(clientReq, "QUIT"));
		printf("client: closing socket\n");
		success = close(sockDes);
		printf("client: socket closed\n");
		validate(success);
	}
	return 0;
}