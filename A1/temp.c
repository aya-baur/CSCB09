#include <string.h>
#include <stdio.h>

#define UT_NAMESIZE 32

int inArray(char** usernames, int lasti, char name[UT_NAMESIZE]){
    //returns 1 if true
    //returns 0 if false
    for(int i=0; i<lasti; i++){
        if(strcmp(usernames[i], name) == 0){
            return 1;
        }
    }
    
    
    return 0;
}

int main(){
    char ut_user[UT_NAMESIZE] = "bauryzh1";
    char *usernames[UT_NAMESIZE] = NULL;
    int lasti = 0;
    if(lasti == 0){
        usernames = ut_user;
        lasti++;
    }
    if(inArray(usernames, lasti, ut_user) == 0){
        //add to array
        usernames[lasti] = ut_user;
    }
}


//copy of user function
void user(){
    struct utmp *users;
    users = getutent();
    setutent();
    int i=0;
    //need an array of chars to store usernames
    //make sure to not double count users that are connected to several sessions
    char* usernames;
    int lasti = 0;
    printf("\n### Sessions/Users ###\n");
    while(users != NULL){
        if(users->ut_type == USER_PROCESS){
            printf("%.*s, ", UT_NAMESIZE, users->ut_name);
            printf("%s, ", users->ut_line);
            printf("%.*s\n", UT_HOSTSIZE,  users->ut_host);
            if(inArray(&usernames, lasti, users->ut_name) == 0){
                //if its not in array, add to array
                *(usernames + lasti) = users->ut_name;
                lasti ++;
            }
            //i++;
        }
        users = getutent();
    }
    printf("There are %d users connected\n", i);
}

