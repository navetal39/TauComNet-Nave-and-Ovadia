#define FILEPATH_LEN 1025
#define DEFAULT_PORT 6423
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_SUBJECT 100
#define MAX_CONTENT 2000
#define MAXEMAILS 32000
#define NUM_OF_CLIENTS 20
#define TOTAL_TO 20

typedef struct mail_t{
    char from[MAX_USERNAME+1];
    user* to[TOTAL_TO];
    int numTo;
    char subject[MAX_SUBJECT+1];
    char content[MAX_CONTENT+1];
} mail;
typedef struct user_t {
    int id;
    char name[MAX_USERNAME+1];
    mail* inbox[MAXEMAILS];
    int isDeleted[MAXEMAILS]; // bool?
} user;

/*
How it works:
mail:	has array of pointers to recievers, no longer has id.
user:	has array of pointers to revelant emails (those that were sent to him).
		mail id = index in array.
		Items are not removed from the array, instead isDeleted is updated when an email is deleted.
*/