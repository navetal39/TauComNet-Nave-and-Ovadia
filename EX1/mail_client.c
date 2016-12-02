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
void handleError(){}
#define sendMgetOK(message) sendRet = sendall(sockDes, (message), strlen(message)+1); validate(sendRet); recvRet = recvall(sockDes, ok); validate(recvRet); //TODO handle? x2
#define validate(var) if((var)==-1){handleError();}

/* Slightly modified code from recitation 2's slides */
int sendall(int sd, char* buf, int len)
{
	int total = 0;
	int n=-1, m=0, bytesleft=len;

	char strnum[11], retnum[11]; // 10 characters (+null terminator) is enough to hold int32's max value. It SHOULD be safe enough...
	memset(strnum, 0, 11);
	memset(retnum, 0, 11);
	sprintf(strnum, "%d", len);
	m=send(sd, strnum, 11, 0)-12;
	validate(m) else{
		m=recv(sd, retnum, 11, 0);
		validate(m) else{
			validate(-(!(!strcmp(retnum, strnum)))) else{
				while(total<len)
				{
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
	int m, n, len, total;
	m = recv(sd, strnum, 11, 0)-12;
	validate(m) else{
		len = atoi(strnum);
		m=send(sd, strnum, 11, 0)-12;
		validate(m) else{
			while(total<len)
			{
				n=recv(sd, buf+total, len-total, 0);
				validate(n) else{
					total += n;
				}
			}
		}
	}
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

int main(int argc, char* argv[])
{
	char welcomeMessage[WELCOME_LENGTH], usernameAndPassword[MAX_USERNAME+MAX_PASSWORD+2], password[MAX_PASSWORD+1], inbox[INBOX_SIZE+1];
	char recps[TOTAL_TO*(MAX_USERNAME+1)], subj[MAX_SUBJECT+1], ctnt[MAX_CONTENT+1], inMail[MAIL_SIZE+1];
	char connected[2], clientReq[18], reqNum[3], nmclReq[8], ok[3];
	int sockDes, recvRet, sendRet, success, numIndex, ar, isGet;
	regex_t nmclReqPattern;
	success = regcomp(&nmclReqPattern, "(GET_MAIL |DELETE_MAIL )[1,9][0,9]*", 0);
	validate(-(!(!success)));
	/* Initialize address struct: */
	struct sockaddr_in serverAddr;
	serverAddr.sin_family=AF_INET;
	if(argc>1) // got ip addr
	{
		success = inet_aton(argv[1], &serverAddr.sin_addr)-1;
		validate(success);
	}else{
		inet_aton("127.0.0.1", &serverAddr.sin_addr);
	}
	if(argc>2) // got port number
	{
		success = sscanf(argv[2], "%hu", &serverAddr.sin_port)-1;
		validate(success);
		serverAddr.sin_port = htons(serverAddr.sin_port);
	}else{
		serverAddr.sin_port = DEFAULT_PORT;
	}

	/* Socket initialization */
	sockDes = socket(PF_INET, SOCK_STREAM, 0);
	success = connect(sockDes, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	validate(success);
	recvRet = recvall(sockDes, welcomeMessage);
	validate(recvRet); //TODO handle?

	/* Authentication */
	printf("%s\n", welcomeMessage);
	readField("User: ");
	success = fgets(usernameAndPassword, MAX_USERNAME, stdin);
	validate(-(!success));
	readField("Password: ");
	success=fgets(password, MAX_PASSWORD, stdin);
	validate(-(!success));
	strcat(usernameAndPassword,"\t");
	strcat(usernameAndPassword,password);
	sendRet = sendall(sockDes, usernameAndPassword, strlen(usernameAndPassword)+1);
	validate(sendRet); //TODO handle?
	recvRet = recvall(sockDes, connected);
	validate(recvRet); //TODO handle?
	if(connected[0] != 'Y')
	{
		printf("Could not connect to the server.\nUsername and password combination is incorrect\n");
		return 0;
	}else{
		printf("Connected to server\n");
		do
		{
			success=fgets(clientReq, 18, stdin);
			validate(-(!success));
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
				sendRet = sendall(sockDes, "4", 2);
				validate(sendRet); //TODO handle?
				success = close(sockDes);
				validate(success);
			}
			else if(strcmp(clientReq,"COMPOSE")==0) // Compose request
			{
				success = readField("To: ");
				validate(success);
				fgets(recps, TOTAL_TO*(MAX_USERNAME+1), stdin);
				success = readField("Subject: ");
				validate(success);
				fgets(subj, MAX_SUBJECT+1, stdin);
				success = readField("Text: ");
				validate(success);
				fgets(ctnt, MAX_CONTENT+1, stdin);
				memset(ok, '\0', 3);
				sendMgetOK("5");
				sendMgetOK("recps");
				sendMgetOK("subj");
				sendMgetOK("ctnt");
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
					if(isGet)
					{
						recvRet = recvall(sockDes, inMail);
						validate(recvRet); //TODO handle?
						printf("%s\n",inMail);
					}
				}
			}
			else
			{
				handleError();
			}
		}while(strcmp(clientReq, "QUIT"));
		success = close(sockDes);
		validate(success);
	}
	return 0;
}