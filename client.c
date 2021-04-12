#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>

#include "msg.h"

#define MAX_USERS 32 // second defined in user.h


int ipc_in_ID,
    ipc_out_ID,
    num_users = 0;


void print_msg(struct msg message){
  if(message.sender==0){
    printf("SERVER: %s \n", message.short_text);
  }
  else{
      printf("[%s] %s: %s\n", message.short_text, message.who, message.long_text);
  }
}


void sign_in(char nick[SIZE_SHORT_TEXT], int password){

  ipc_out_ID = msgget(0x1337, 0600 | IPC_CREAT);
  ipc_in_ID = msgget(getpid() *1337, 0600 | IPC_CREAT);


  struct msg login_msg, in_msg;
  login_msg.priority = 10;
  login_msg.type = LOGIN;
  login_msg.sender = getpid();
  login_msg.number = password;
  strcpy(login_msg.short_text, nick);

  msgsnd(ipc_out_ID, &login_msg, MSG_SIZE, IPC_NOWAIT);
  msgrcv(ipc_in_ID, &in_msg,MSG_SIZE, 10, 0);

  int n_trials = 4;
  while(in_msg.type == FAILURE){

    n_trials--;
    if(n_trials>0){
      print_msg(in_msg);
      printf("Wrong password! %d trials left\n PASSWORD: \n", n_trials);

      struct msg login_msg;
      login_msg.priority = 10;
      login_msg.type = LOGIN;
      login_msg.sender = getpid();
      strcpy(login_msg.short_text, nick);
      scanf("%d", &login_msg.number);
      msgsnd(ipc_out_ID, &login_msg, MSG_SIZE, 0);
      msgrcv(ipc_in_ID, &in_msg, MSG_SIZE, 10, 0);
    }
    else{
      printf("Wrong password! you are disconnected \n");
      kill(getpid(), SIGKILL);
    }
  }
  print_msg(in_msg);
}


void print_help(){
  printf("        ### H E L P   P A G E ###\n");
  printf("Use /<command> [<arg>] for navigate in message system.\n");
  printf("  /help                           - help page.\n");
  printf("  /register <subject_name>        - register new subject (if it does not exist!).\n");
  printf("  /join <subject_name> <n>        - join to subject temporarily(n msg) or permament (n = -1).\n");
  printf("  /send <subject_name> <msg>      - send public msg to subject.\n");
  printf("  /startread                      - start read messages asynchronously.\n");
  printf("  /stopread                       - stop read messages asynchronously.\n");
  printf("  /read                           - to read one message.\n");
  printf("  /mute <user_name>               - block msg from user.\n");
  printf("  /mypid                          - check my pid.\n");
  printf("  /logout                         - quit chat.\n");
}


void server_reader(){
  while(1){
    sleep(0.1);
    struct msg in_msg;
    msgrcv(ipc_in_ID, &in_msg, MSG_SIZE, 10, 0);
    if(in_msg.type == NEW_SUBJECT){
      printf("SERVER: New subject: %s \n", in_msg.short_text);
    }
    else if(in_msg.type == NEW_USER){
      printf("SERVER: %s is active\n", in_msg.short_text);
    }
    else{
      print_msg(in_msg);
    }
  }
}


void asynch_reader(){
  while(1){
      sleep(0.1);
    for(int priority = 9; priority > 0; priority--){
      struct msg in_msg;
      if (msgrcv(ipc_in_ID, &in_msg, MSG_SIZE, priority, IPC_NOWAIT) > 0){
          print_msg(in_msg);
          priority = 9;
      }
    }
  }
}


int main(int argc, char *argv[]){
  if(argc == 3){
    sign_in(argv[1], atoi(argv[2]));
  }
  else{
    printf("Input nick as first argument and password (number) as seccond\n");
    return 1;
  }

  printf("Use /<command> [<arg>] for navigate. Example: /help\n");


char input[SIZE_SHORT_TEXT];
int reading_process_pid = 0,
    server_reading_process_pid;

if((server_reading_process_pid = fork()) == 0){
  server_reader();
}

while(1){
  strcpy(input, "");
  scanf("%s", input);

  if(strstr(input, "/help") != NULL){
      print_help();
  }
  else if(strstr(input, "/register") != NULL){
      strcpy(input, "");
      struct msg register_msg;

      register_msg.priority = 10;
      register_msg.type = REGISTER;
      register_msg.sender = getpid();
      scanf("%s", register_msg.short_text);

      msgsnd(ipc_out_ID, &register_msg, sizeof(register_msg), 0);
  }

  else if(strstr(input, "/join") != NULL){
    strcpy(input, "");
    struct msg join_msg;

    scanf("%s", join_msg.short_text);
    join_msg.priority = 10;
    join_msg.type = JOIN;
    join_msg.sender = getpid();
    scanf("%d", &join_msg.number);

    msgsnd(ipc_out_ID, &join_msg, sizeof(join_msg), 0);
  }

  else if(strstr(input, "/send") != NULL){
    strcpy(input, "");
    struct msg message;

    scanf("%s", message.short_text);
    int priority;
    scanf("%d", &priority);
    message.priority = check_priority(priority);
    message.sender = getpid();
    message.type = MSG;

    fgets(message.long_text,SIZE_LONG_TEXT, stdin);
    msgsnd(ipc_out_ID, &message, sizeof(message), 0);
  }

  else if(strstr(input, "/startread") != NULL){

    if(reading_process_pid == 0){
      if((reading_process_pid = fork()) == 0){
        asynch_reader();
        kill(reading_process_pid, SIGKILL);
        exit(0);
      }
      printf("Now messages are reading asynchronously.\n");
    }
    else{
      printf("You are reading messages asynchronously!\n");
    }
  }

  else if(strstr(input, "/stopread") != NULL){

    if(reading_process_pid > 0){
      kill(reading_process_pid, SIGKILL);
      reading_process_pid = 0;
      printf("Now messages can be read by \"/read.\"\n");
    }
    else{
      printf("You are reading messages synchronously!\n");
    }
  }

  else if(strstr(input, "/read") != NULL){
    strcpy(input, "");
    if(reading_process_pid > 0){
      printf("Use  \"/stopread.\" to read messages by \"/read.\" \n");
    }
    else{
      // read_msg();
      struct msg in_msg;
      for(int priority = 9; priority > 0; priority--){
        if (msgrcv(ipc_in_ID, &in_msg, MSG_SIZE, priority, IPC_NOWAIT) > 0){
            print_msg(in_msg);
          break;
        }
      }
    }
  }
  else if(strstr(input, "/mute") != NULL){
    strcpy(input, "");
    struct msg mute_msg;

    scanf("%s", mute_msg.short_text);
    mute_msg.priority = 10;
    mute_msg.type = MUTE;
    mute_msg.sender = getpid();

    msgsnd(ipc_out_ID, &mute_msg, sizeof(mute_msg), 0);
  }
  else if(strstr(input, "/mypid") != NULL){
    printf("Your PID is: %d\n", getpid());
  }
  else if(strstr(input, "/logout") != NULL){
    if(reading_process_pid > 0){
      kill(reading_process_pid, SIGKILL);
    }
    kill(server_reading_process_pid, SIGKILL);
    return 0;
  }
}

return 0;
}
