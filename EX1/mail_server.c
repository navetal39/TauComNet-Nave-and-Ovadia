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
#define MAX_LOGIN MAX_USERNAME+MAX_PASSWORD+2
#define INBOX_SIZE (9+MAX_USERNAME+MAX_SUBJECT)*MAXEMAILS+1
#define MAIL_SIZE 29+(MAX_USERNAME*(TOTAL_TO+1))+MAX_SUBJECT+MAX_CONTENT

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


int main(int argc, char *argv[]){
        printf("FIRST\n");
        fflush(stdout);
        char filepath[FILEPATH_LEN],command[8], username[MAX_USERNAME+1],userAndPassword[MAX_LOGIN],cur[1];
        int srvPort = DEFAULT_PORT,clientSock,auth = 0,curEmails = 0,curTo,myEmailAmount = 0, found = 0, count = 0, count2 = 0, curUsers = 0;
        FILE *users_file;
        printf("after FILE\n");
        fflush(stdout);
        struct sockaddr_in srvAddr, clientAddr;
        static struct mail emails[MAXEMAILS], myEmails[MAXEMAILS];
        struct user userList[NUM_OF_CLIENTS];
        printf("After structs\n");
        char fileUserPass[MAX_LOGIN];
        size_t addrSize = sizeof(clientAddr);


        /* Checking input to see if non-default port was received */
        if(argc>3||argc<2){
                printf("Illegal Usage: Should be mail_server <users_file> [port]\n");
                return 1;
        }
        strcpy(filepath,argv[1]);
        printf("HELLO\n");
        fflush(stdout);
        users_file = fopen(filepath, "r");
        printf("file opened\n");
        fflush(stdout);
        if(argc == 3){
                srvPort =atoi(argv[2]);
        }
        /* Initialize server socket */
        int srvSock = socket(PF_INET,SOCK_STREAM,0);
        printf("socket created\n");
        fflush(stdout);
        srvAddr.sin_family = AF_INET;
        srvAddr.sin_port = htons(srvPort);
        srvAddr.sin_addr.s_addr = INADDR_ANY;
        bind(srvSock,(struct sockaddr*)&srvAddr,sizeof(srvAddr));
        printf("socket bound\n");
        fflush(stdout);

        /* Generate user list */
        fread(cur,1,1,users_file);
        while (*cur != EOF && count < NUM_OF_CLIENTS){
                if(!strcmp(cur,"\t")){
                        while(strcmp(cur,"\n")){
                                fread(cur,1,1,users_file);
                        }
                        username[count2] = 0;
                        strcpy(userList[curUsers].name,username);
                        userList[curUsers].id = 0;
                        ++curUsers;
                        count2 = 0;
                }
                else{
                        username[count2] = *cur;
                        ++count2;
                }

        }
        printf("User list generated\n");
        fflush(stdout);
        fseek(users_file,0,SEEK_SET);

        /* Server operation loop */
        while (1){
                listen(srvSock,1);
                clientSock = accept(srvSock,(struct sockaddr*) &clientAddr,(socklen_t *__restrict__) &addrSize);
                if(clientSock != -1){
                        send(clientSock,WELCOME, sizeof(WELCOME),0);
                        recv(clientSock,&userAndPassword,MAX_LOGIN,0);



                        /* Check user file for authentication */
                        while(fread(cur,1,1,users_file)!= EOF){
                                while(count<MAX_LOGIN+1 && strcmp(cur,"\n")){
                                        fileUserPass[count] = *cur;
                                        ++count;
                                        fread(cur,1,1,users_file);
                                }
                                fileUserPass[count +1] = '\0';
                                if(strcmp(userAndPassword,fileUserPass) == 0){
                                        send(clientSock,"Y",1,0);
                                        auth = 1;
                                }

                        }
                        if(!auth){
                                send(clientSock,"N",1,0);
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
                        recv(clientSock,&command,8,0);
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
                                        send(clientSock,inbox,INBOX_SIZE,0);

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
                                                send(clientSock,reqMail,MAIL_SIZE,0);
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
                                }

                                /* Case "Compose" */
                                else if(!strcmp(&command[0],"5")){
                                        send(clientSock,"OK",2,0); //Ready to receive compose data
                                        struct mail newMail;
                                        strcpy(newMail.from,username);
                                        char toName[MAX_USERNAME+1][TOTAL_TO], toSubj[MAX_SUBJECT+1], toText[MAX_CONTENT+1];
                                        recv(clientSock,&toName,(MAX_USERNAME+1)*TOTAL_TO,0);
                                        char *token;
                                        count = 0;
                                        while((token = strtok(NULL,",")) != NULL){
                                                for(count2 = 0; count2 < curUsers; ++count2){
                                                        if(!strcmp(token,userList[count2].name)){
                                                                newMail.to[count].id = (++userList[count2].id);
                                                                strcpy(newMail.to[count].name,token);
                                                                ++count;
                                                                break;
                                                        }
                                                }
                                        }
                                        recv(clientSock,&toSubj,MAX_SUBJECT+1,0);
                                        strcpy(newMail.subject,toSubj);
                                        recv(clientSock,&toText,MAX_CONTENT+1,0);
                                        strcpy(newMail.content,toText);
                                        send(clientSock,"OK",2,0);
                                        emails[++curEmails] = newMail;

                                }

                        }

                        close(clientSock);

                }
        }


}
