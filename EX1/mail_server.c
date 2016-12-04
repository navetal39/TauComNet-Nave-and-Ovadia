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

struct user {
    char name[MAX_USERNAME + 1];
    int id;
};

struct mail {
    char from[MAX_USERNAME + 1];
    struct user to[TOTAL_TO];
    int numTo;
    char subject[MAX_SUBJECT + 1];
    char content[MAX_CONTENT + 1];
    int tempID;
};

void handleError() { printf("An error occured\n"); }

/* Slightly modified code from recitation 2's slides */
int sendall(int sd, char* buf, int len)
{
    int total = 0;
    int n = -1, m = 0, bytesleft = len;

    char strnum[11], retnum[11]; // 10 characters (+null terminator) is enough to hold int32's max value. It SHOULD be safe enough...
    memset(strnum, 0, 11);
    memset(retnum, 0, 11);
    sprintf(strnum, "%d", len);
    m = send(sd, strnum, 11, 0) - 11;
    validate(-(!(!m))) else {
        m = recv(sd, retnum, 11, 0);
        validate(m) else {
            validate(-(!(!strcmp(retnum, strnum)))) else {
                while (total < len)
                {
                    n = send(sd, buf + total, bytesleft, 0);
                    validate(n) else {
                        total += n;
                        bytesleft -= n;
                    }
                }
            }
        }
    }
    return n == -1 ? -1 : 0;
}

int recvall(int sd, char* buf)
{
    char strnum[11];
    memset(strnum, 0, 11);
    int m, m2, n = -1, len, total = 0;
    m = recv(sd, strnum, 11, 0) - 11;
    validate(m) else {
        len = atoi(strnum);
        m2 = send(sd, strnum, m + 11, 0) - (m + 11);
        validate(-(!(!m2))) else {
            while (total < len)
            {
                n = recv(sd, buf + total, len - total, 0);
                validate(n) else {
                    total += n;
                }
            }
        }
    }
    return n == -1 ? -1 : 0;
}

int main(int argc, char *argv[])
{
    char filepath[FILEPATH_LEN], command[8], username[MAX_USERNAME + 1], username2[MAX_USERNAME + 1], password[MAX_PASSWORD + 1], password2[MAX_PASSWORD + 1], cur[2];
    int srvPort = htons(DEFAULT_PORT), clientSock, auth = 0, curEmails = 0, curTo, myEmailAmount = 0, found = 0, count = 0, count2 = 0, curUsers = 0, mailID;
    FILE *users_file;
    struct sockaddr_in srvAddr;
    static struct mail emails[MAXEMAILS], myEmails[MAXEMAILS];
    struct user userList[NUM_OF_CLIENTS];

    /* Checking input to see if non-default port was received */
    if (argc > 3 || argc < 2)
    {
        printf("Illegal Usage: Should be mail_server <users_file> [port]\n");
        return 1;
    }
    strcpy(filepath, argv[1]);
    users_file = fopen(filepath, "r");
    if (argc == 3)
    {
        srvPort = htons(atoi(argv[2]));
    }

    /* Initialize server socket */
    int srvSock = socket(PF_INET, SOCK_STREAM, 0);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = srvPort;
    srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(srvSock, (struct sockaddr*)&srvAddr, sizeof(srvAddr));


    /* Generate user list */
    while (fscanf(users_file, "%s\t%s\n", username, password) != EOF)
    {
        strcpy(userList[curUsers].name, username);
        userList[curUsers].id = 0;
        ++curUsers;
    }
    fseek(users_file, 0, SEEK_SET);


    /* Server operation loop */
    listen(srvSock, 1);
    while (1)
    {
        memset(username, 0, MAX_USERNAME + 1);
        memset(password, 0, MAX_PASSWORD + 1);
        memset(username2, 0, MAX_USERNAME + 1);
        memset(password2, 0, MAX_PASSWORD + 1);
        memset(cur, 0, 2);
        memset(myEmails, 0, MAXEMAILS * sizeof(struct mail));
        myEmailAmount = 0;
        clientSock = accept(srvSock, (struct sockaddr*)NULL, NULL);
        if (clientSock != -1)
        {
            sendall(clientSock, WELCOME, sizeof(WELCOME));
            recvall(clientSock, username2);
            sendall(clientSock, "OK", 3);
            recvall(clientSock, password2);

            /* Check user file for authentication */
            while (fscanf(users_file, "%s\t%s\n", username, password) != EOF)
            {
                if (!strcmp(username, username2) && !strcmp(password, password2))
                {
                    sendall(clientSock, "Y", 1);
                    auth = 1;
                    break;
                }
            }
            fseek(users_file, 0, SEEK_SET);
            if (!auth)
            {
                sendall(clientSock, "N", 1);
                close(clientSock);
                continue;
            }

            /* Load current user Emails for easy access */
            for (count = 0; count < curEmails; ++count)
            {
                for (curTo = 0; curTo < emails[count].numTo; ++curTo) {
                    if (!strcmp(emails[count].to[curTo].name, username) && emails[count].to[curTo].id != 0)
                    {
                        myEmails[myEmailAmount] = emails[count];
                        myEmails[myEmailAmount].tempID = emails[count].to[curTo].id;
                        ++myEmailAmount;
                        break;
                    }
                }
            }

            /* Receive commands from client and execute */
            recvall(clientSock, command);
            while (command[0] != '4')
            {
                mailID = 0;
                /* Case "Show Inbox" */
                if (command[0] == '1')
                {
                    char forID[10];
                    char inbox[INBOX_SIZE + 1];
                    memset(inbox, 0, INBOX_SIZE);
                    for (count = 0; count < myEmailAmount; ++count)
                    {
                        if (myEmails[count].tempID != -1)
                        {
                            sprintf(forID, "%d", myEmails[count].tempID);
                            strcat(inbox, forID);
                            strcat(inbox, " ");
                            strcat(inbox, myEmails[count].from);
                            strcat(inbox, " \"");
                            strcat(inbox, myEmails[count].subject);
                            strcat(inbox, "\"\n");
                        }
                    }
                    strcat(inbox, "\0");
                    inbox[strlen(inbox) - 1] = '\0';
                    if (strlen(inbox) == 0)
                    {
                        sendall(clientSock, "EMPTY", 6);
                    } else {
                        sendall(clientSock, inbox, strlen(inbox));
                    }
                }

                /* Case "Get Mail" */
                else if (command[0] == '2')
                {
                    found = 0;
                    char reqMail[MAIL_SIZE];
                    memset(reqMail, 0, MAIL_SIZE);
                    for (count = 0; (count < myEmailAmount) && !found; ++count)
                    {
                        sscanf(command, "2 %d", &mailID);
                        if (myEmails[count].tempID == mailID)
                        {
                            strcat(reqMail, "From: ");
                            strcat(reqMail, myEmails[count].from);
                            strcat(reqMail, "\n");
                            strcat(reqMail, "To: ");
                            for (curTo = 0; curTo < myEmails[count].numTo; ++curTo)
                            {
                                if (curTo > 0)
                                {
                                    strcat(reqMail, ",");
                                }
                                strcat(reqMail, myEmails[count].to[curTo].name);
                            }
                            strcat(reqMail, "\n");
                            strcat(reqMail, "Subject: ");
                            strcat(reqMail, myEmails[count].subject);
                            strcat(reqMail, "\n");
                            strcat(reqMail, "Text: ");
                            strcat(reqMail, myEmails[count].content);
                            sendall(clientSock, reqMail, MAIL_SIZE);
                            found = 1;
                        }
                    }
                    if (!found)
                    {
                        sendall(clientSock, "ERROR: Mail not found.", MAIL_SIZE);
                    }
                }

                /* Case  "Delete Mail" */
                else if (command[0] == '3')
                {
                    sscanf(command, "3 %d", &mailID);
                    found = 0;
                    for (count = 0; (count < myEmailAmount) && !found; ++count)
                    {
                        if (myEmails[count].tempID == mailID)
                        {
                            myEmails[count].tempID = -1;
                            for (curTo = 0; (curTo < myEmails[count].numTo) && !found; ++curTo)
                            {
                                if (!strcmp(myEmails[count].to[curTo].name, username))
                                {
                                    myEmails[count].to[curTo].id = -1;
                                    found = 1;
                                }
                            }
                        }
                    }

                    sendall(clientSock, "OK", 2);
                }

                /* Case "Compose" */
                else if (command[0] == '5')
                {
                    sendall(clientSock, "OK", 2); //Ready to receive compose data
                    struct mail newMail;
                    memset(&newMail, 0, sizeof(struct mail));
                    strcpy(newMail.from, username);
                    char toName[(MAX_USERNAME + 1)*TOTAL_TO], toSubj[MAX_SUBJECT + 1], toText[MAX_CONTENT + 1];
                    recvall(clientSock, toName);
                    char *token;
                    count = 0;
                    found = 0;
                    token = strtok(toName, ",");
                    while (token != NULL)
                    {
                        for (count2 = 0; count2 < curUsers; ++count2)
                        {
                            if (!strcmp(token, userList[count2].name))
                            {
                                count = 1;
                                strcpy(newMail.to[newMail.numTo].name, token);
                                ++(userList[count2].id);
                                newMail.to[newMail.numTo].id = userList[count2].id;
                                ++(newMail.numTo);
                                if (!strcmp(token, username))
                                {
                                    found = 1;
                                }
                            }
                        }
                        token = strtok(NULL, ",");
                    }
                    sendall(clientSock, "OK", 2);
                    recvall(clientSock, toSubj);
                    strcpy(newMail.subject, toSubj);
                    sendall(clientSock, "OK", 2);
                    recvall(clientSock, toText);
                    strcpy(newMail.content, toText);
                    sendall(clientSock, "OK", 2);
                    if (count)
                    {
                        emails[curEmails] = newMail;
                        ++curEmails;
                        if (found)
                        {
                            myEmails[myEmailAmount] = newMail;
                            myEmails[myEmailAmount].tempID = (myEmailAmount + 1);
                            ++myEmailAmount;
                        }
                    }
                }
                recvall(clientSock, command);
            }
            close(clientSock);
        }
    }
}
