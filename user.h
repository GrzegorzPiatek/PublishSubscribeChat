#ifndef USERS_H
#define USERS_H
#define MAX_USERS 32


struct user{
  char nick[SIZE_SHORT_TEXT];
  int pid;
  int password;
  int ipc_ID;
  int muted[MAX_USERS];
} Users[MAX_USERS];


//return ipc queue id with gived user
int find_user_ipc_ID(const int user_pid, int *num_users){
  for(int i = 0; i <= *num_users; i++){
    if(Users[i].pid == user_pid){
      return Users[i].ipc_ID;
    }
  }
  return -1;
}


char* find_user_nick(struct user Users[MAX_USERS], int user_pid, int num_users){

  for(int i = 0; i <= num_users; i++){
    if(Users[i].pid == user_pid){
      return Users[i].nick;
    }
  }
  return "";
}


// return index of user with user_nick, else return -1
int check_user_nick(const char *user_nick, int *num_users){
  for(int i = 0; i <= *num_users; i++){
    if(strstr(Users[i].nick, user_nick) != NULL){
      return i;
    }
  }
    return -1;
}


void print_users(struct user *Users, int *num_users){
  printf("Active users (%d)\n", *num_users);
  for(int i = 0; i < *num_users; i++){
    printf("ID: %d, Nick: %s\n", Users[i].pid, Users[i].nick);
  }
}
#endif
