#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define FILEPATH_LEN 1025
#define DEFAULT_PORT 6423
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_SUBJECT 100
#define MAX_CONTENT 2000
#define MAXEMAILS 32000
#define NUM_OF_CLIENTS 20
#define TOTAL_TO 20
#define WELCOME "Connection established. Welcome!"
#define MAX_LOGIN MAX_USERNAME+MAX_PASSWORD+3
#define INBOX_SIZE (9+MAX_USERNAME+MAX_SUBJECT)*MAXEMAILS+1
#define MAIL_SIZE 29+(MAX_USERNAME*(TOTAL_TO+1))+MAX_SUBJECT+MAX_CONTENT
#define validate(var) if((var)==-1){handleError();}

struct user{
        char name[MAX_USERNAME+1];
        int id;
};

struct mail{
        char from[MAX_USERNAME+1];
        struct user to[TOTAL_TO];
        int numTo;
        char subject[MAX_SUBJECT+1];
        char content[MAX_CONTENT+1];
        int tempID;
};

int sendall(int sd, char* buf, int len);
int recvall(int sd, char* buf);
void handleError(){printf("SERVER: shit's on fire, yo!!!!!!\n");}


int main(int argc, char *argv[]){
        char filepath[FILEPATH_LEN],command[8], username[MAX_USERNAME+1],userAndPassword[MAX_LOGIN],cur[2];
        int srvPort = htons(DEFAULT_PORT),clientSock,auth = 0,curEmails = 0,curTo,myEmailAmount = 0, found = 0, count = 0, count2 = 0, curUsers = 0;
        FILE *users_file;
        struct sockaddr_in srvAddr, clientAddr;
        static struct mail emails[MAXEMAILS], myEmails[MAXEMAILS];
        struct user userList[NUM_OF_CLIENTS];
        char fileUserPass[MAX_LOGIN];
        size_t addrSize = sizeof(clientAddr);


        /* Checking input to see if non-default port was received */
        if(argc>3||argc<2){
                printf("Illegal Usage: Should be mail_server <users_file> [port]\n");
                return 1;
        }
        strcpy(filepath,argv[1]);
        users_file = fopen(filepath, "r");
        if(argc == 3){
                srvPort = htons(atoi(argv[2]));
        }
        /* Initialize server socket */
        printf("SERVER: initializing server socket\n");
        int srvSock = socket(PF_INET,SOCK_STREAM,0);
        srvAddr.sin_family = AF_INET;
        srvAddr.sin_port = srvPort;
        srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        printf("SERVER: binding server socket\n");
        bind(srvSock,(struct sockaddr*)&srvAddr,sizeof(srvAddr));

        printf("SERVER: reading users file\n");
        /* Generate user list */
        fread(cur,1,1,users_file);
        while (!feof(users_file) && count < NUM_OF_CLIENTS){
                if(!strcmp(cur,"\t")){
                        while(strcmp(cur,"\n")){
                                fread(cur,1,1,users_file);
                        }
                        username[count2] = 0;
                        strcpy(userList[curUsers].name,username);
                        userList[curUsers].id = 0;
                        ++curUsers;
                        ++count;
                        count2 = 0;
                }
                else{
                        strncpy(&username[count2],cur,1);
                        ++count2;
                        fread(cur,1,1,users_file);
                }
        }
        fseek(users_file,0,SEEK_SET);

        printf("SERVER: ready for clients\n");
        /* Server operation loop */
        getsockname(srvSock, (struct sockaddr*)&srvAddr, addrSize);
        printf("SERVER: port number: %d, %d, %d\n", ntohs(srvAddr.sin_port), srvAddr.sin_port, srvPort);
        listen(srvSock,2);
        while (1){
                printf("SERVER: listening\n"); //V NULL, NULL -> &clientAddr, (socklen_t *__restrict__) &addrSize
                clientSock = accept(srvSock,(struct sockaddr*)NULL, NULL);
                printf("SERVER: accepted client\n");
                if(clientSock != -1){
                        printf("SERVER: sending welcome\n");
                        sendall(clientSock,WELCOME, sizeof(WELCOME));
                        printf("SERVER: getting authentication information\n");
                        recvall(clientSock,userAndPassword);
                        printf("SERVER: authentication info obtained:\n-%s-%d-\n", userAndPassword, strlen(userAndPassword));

                        /* Check user file for authentication */
                        fgets(cur,1,users_file);
                        while(!feof(users_file)){
                            
                                while(count<MAX_LOGIN+1 && strcmp(cur,"\n")){
                                	printf("%s,%d\n",cur,strcmp(cur,"\n"));
                                    fileUserPass[count] = *cur;
                                    ++count;
                                    fgets(cur,1,users_file);
                                }
                                printf("%d,%s\n",strlen(fileUserPass),fileUserPass);
                                fileUserPass[count +1] = '\n';
                                fileUserPass[count + 2] = '\0';
                                printf("OPOPOPOP %d,%s\n",strlen(userAndPassword), userAndPassword);
                                if(strcmp(userAndPassword,fileUserPass) == 0){
                                        sendall(clientSock,"Y",1);
                                        auth = 1;
                                }
                                fgets(cur,1,users_file);
                        }
                        if(!auth){
                                printf("SERVER: incorrect auth info\n");
                                sendall(clientSock,"N",1);
                                close(clientSock);
                                continue;
                        }

                        count = 0;
                        strcpy(cur,&userAndPassword[0]);
                        while(strcmp(cur,"\t")){
                                username[count] = *cur;
                                ++count;
                                *cur = userAndPassword[count];
                        }
                        username[count] = '\0';

                        /* Load current user Emails for easy access */
                        for(count = 0;count<curEmails;++count){
                                for(curTo = 0; curTo<emails[count].numTo;++curTo){
                                        if(!strcmp(emails[count].to[curTo].name, username) && emails[count].to[curTo].id != 0){
                                                myEmails[myEmailAmount] = emails[count];
                                                myEmails[myEmailAmount].tempID = emails[count].to[curTo].id;
                                                ++myEmailAmount;
                                                break;
                                        }
                                }
                        }

                        /* Receive commands from client and execute */
                        recvall(clientSock,command);
                        while(!strcmp(&command[0],"4")){


                                /* Case "Show Inbox" */
                                if(!strcmp(&command[0], "1")){
                                        char inbox[INBOX_SIZE];
                                        memset(inbox,0,INBOX_SIZE);
                                        for(count = 0;count<myEmailAmount;++count){
                                                if(myEmails[count].tempID != 0){
                                                        strcat(inbox,(char *)&myEmails[count].to[curTo].id);
                                                        strcat(inbox," ");
                                                        strcat(inbox, myEmails[count].from);
                                                        strcat(inbox, " ");
                                                        strcat(inbox, myEmails[count].subject);
                                                        strcat(inbox, "\n");
                                                }
                                        }
                                        sendall(clientSock,inbox,INBOX_SIZE);

                                }

                                /* Case "Get Mail" */
                                else if(!strcmp(&command[0],"2")){
                                        found = 0;
                                        char reqMail[MAIL_SIZE];
                                        memset(reqMail,0,MAIL_SIZE);
                                        for(count = 0; (count < myEmailAmount) && !found; ++count){
                                                if(myEmails[count].tempID == atoi((command+2))){
                                                        strcat(reqMail,"From: ");
                                                        strcat(reqMail,myEmails[count].from);
                                                        strcat(reqMail,"\n");
                                                        strcat(reqMail,"To: ");
                                                        for(curTo = 0; curTo<myEmails[count].numTo;++curTo){
                                                                if(curTo>0){
                                                                        strcat(reqMail,",");
                                                                }
                                                                strcat(reqMail,myEmails[count].to[curTo].name);
                                                        }
                                                        strcat(reqMail,"\n");
                                                        strcat(reqMail,"Text: ");
                                                        strcat(reqMail,myEmails[count].content);
                                                        strcat(reqMail,"\n");
                                                }
                                                sendall(clientSock,reqMail,MAIL_SIZE);
                                                found = 1;
                                        }
                                }


                                /* Case  "Delete Mail" */
                                else if(!strcmp(&command[0],"3")){
                                        found = 0;
                                        for(count = 0; (count < myEmailAmount) && !found; ++count){
                                                if(myEmails[count].tempID == atoi((command+2))){
                                                        for(curTo = 0; (curTo < myEmails[count].numTo) && !found; ++curTo){
                                                                if(!strcmp(myEmails[count].to[curTo].name,username)){
                                                                        myEmails[count].to[curTo].id = 0;
                                                                        found = 1;
                                                                        
                                                                }
                                                        }
                                                }
                                        }
                                        sendall(clientSock,"OK",2);
                                        
                                }

                                /* Case "Compose" */
                                else if(!strcmp(&command[0],"5")){
                                        sendall(clientSock,"OK",2); //Ready to receive compose data
                                        struct mail newMail;
                                        strcpy(newMail.from,username);
                                        char toName[(MAX_USERNAME+1)*TOTAL_TO], toSubj[MAX_SUBJECT+1], toText[MAX_CONTENT+1];
                                        recvall(clientSock,toName);
                                        char *token;
                                        count = 0;
                                        token = strtok(toName,",");
                                        while((token != NULL)){
                                                for(count2 = 0; count2 < curUsers; ++count2){
                                                        if(!strcmp(token,userList[count2].name)){
                                                                newMail.to[count].id = (++userList[count2].id);
                                                                strcpy(newMail.to[count].name,token);
                                                                ++count;
                                                                break;
                                                        }
                                                }
                                                token = strtok(NULL,",");
                                        }
                                        sendall(clientSock,"OK",2);
                                        recvall(clientSock,toSubj);
                                        strcpy(newMail.subject,toSubj);
                                        sendall(clientSock,"OK",2);
                                        recvall(clientSock,toText);
                                        strcpy(newMail.content,toText);
                                        sendall(clientSock,"OK",2);
                                        if(count){
                                                emails[++curEmails] = newMail;        
                                        }
                                        

                                }
                                recvall(clientSock,command);

                        }

                        close(clientSock);

                }
        }


}
int sendall(int sd, char* buf, int len)
{
        printf("SERVER: trying to send %s\n", buf);
        int total = 0;
        int n=-1, m=0, bytesleft=len;

        char strnum[11], retnum[11]; // 10 characters (+null terminator) is enough to hold int32's max value. It SHOULD be safe enough...
        memset(strnum, 0, 11);
        memset(retnum, 0, 11);
        sprintf(strnum, "%d", len);
        m=send(sd, strnum, 11, 0)-11;
        printf("SERVER: sent length of %s\n", strnum);
        validate(-(!(!m))) else{
                m=recv(sd, retnum, 11, 0);
                validate(m) else{
                        validate(-(!(!strcmp(retnum, strnum)))) else{
                                while(total<len)
                                {
                                        printf("SERVER: sending %s\n", buf+total);
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
int recvall(int sd, char* buf)
{
        char strnum[11];
        memset(strnum, 0, 11);
        int m, m2, n=-1, len, total = 0;
        m = recv(sd, strnum, 11, 0)-11;
        printf("SERVER: got %d characters\n", m+11);
        validate(m) else{
                printf("SERVER: got length: %s\n", strnum);
                len = atoi(strnum);
                m2=send(sd, strnum, m+11, 0)-(m+11);
                printf("SERVER: sent %d characters\n", m+11);
                validate(-(!(!m2))) else{
                        printf("SERVER: getting data\n");
                        while(total<len)
                        {
                                printf("SERVER: recieving... got %d out of %d\n", total, len);
                                n=recv(sd, buf+total, len-total, 0);
                                validate(n) else{
                                        total += n;
                                }
                        }
                }
        }
        return n==-1?-1:0;
}

