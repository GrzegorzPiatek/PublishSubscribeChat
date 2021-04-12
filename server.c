#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>

#include "msg.h"
#include "user.h"
#include "subject.h"


int num_users = 0,
    num_subjects = 0;


void send_success(const int user_pid){
  struct msg msg_back;
  msg_back.priority = 10;
  msg_back.type = SUCCESS;
  msg_back.sender = 0;
  strcpy(msg_back.short_text, "Success!");

  int ipc_ID = find_user_ipc_ID(user_pid, &num_users);
  msgsnd(ipc_ID, &msg_back, sizeof(msg_back), 0);
}


void send_failure(const int user_pid){
  struct msg msg_back;
  msg_back.priority = 10;
  msg_back.type = FAILURE;
  msg_back.sender = 0;
  strcpy(msg_back.short_text, "Failure!");

  int ipc_ID = msgget(user_pid*1337, 0600 | IPC_CREAT);
  msgsnd(ipc_ID, &msg_back, sizeof(msg_back), 0);
}


void send_new_subject(const char *name){
  struct msg msg_back;
  msg_back.priority = 10;
  msg_back.type = NEW_SUBJECT;
  msg_back.sender = 0;
  strcpy(msg_back.short_text, name);

  for (int i = 0; i < num_users; i++){
    msgsnd(Users[i].ipc_ID, &msg_back, sizeof(msg_back), 0);
  }
}


void send_all_subjects(const int user_pid){
  struct msg msg_back;
  msg_back.priority = 10;
  msg_back.type = NEW_SUBJECT;
  msg_back.sender = 0;

  int ipc_ID = find_user_ipc_ID(user_pid, &num_users);
  for(int i = 0; i<num_subjects; i++){
    strcpy(msg_back.short_text, "");
    strcpy(msg_back.short_text, Subjects[i].name);
    msgsnd(ipc_ID, &msg_back, sizeof(msg_back), 0);
  }
}

void send_all_users(const int user_pid){
  struct msg msg_back;
  msg_back.priority = 10;
  msg_back.type = NEW_USER;
  msg_back.sender = 0;

  int ipc_ID = find_user_ipc_ID(user_pid, &num_users);
  for(int i = 0; i<num_users-1; i++){
    strcpy(msg_back.short_text, "");
    strcpy(msg_back.short_text, Users[i].nick);
    msgsnd(ipc_ID, &msg_back, sizeof(msg_back), 0);
  }
}

void send_new_user(const char name[SIZE_SHORT_TEXT]){
  struct msg msg_back;
  msg_back.priority = 10;
  msg_back.type = NEW_USER;
  msg_back.sender = 0;
  strcpy(msg_back.short_text, name);

  for (int i = 0; i < num_users-1; i++){
    msgsnd(Users[i].ipc_ID, &msg_back, sizeof(msg_back), 0);
  }
}


void add_user(struct msg login_msg){
  int user_index;

  if((user_index = check_user_nick(login_msg.short_text, &num_users)) >= 0){

    if(Users[user_index].password == login_msg.number){
      strcpy(Users[user_index].nick, login_msg.short_text);
      Users[user_index].pid = login_msg.sender;
      Users[user_index].ipc_ID = msgget(Users[user_index].pid *1337, 0600 | IPC_CREAT);
      send_success(Users[user_index].pid);
      send_all_subjects(login_msg.sender);
      send_all_users(login_msg.sender);
      printf("# user no%d [%s] is now online\n", user_index, Users[user_index].nick);
    }
    else{
      send_failure(login_msg.sender);
      printf("@ User no%d [%s] CAN'T login \n", user_index, login_msg.short_text);
    }
  }
  else if(num_users < MAX_USERS){
    strcpy(Users[num_users].nick, login_msg.short_text);
    Users[num_users].pid = login_msg.sender;
    Users[num_users].ipc_ID = msgget(login_msg.sender*1337, 0600 | IPC_CREAT);
    Users[num_users].password = login_msg.number;
    Users[num_users].muted[num_users] = 1;
    num_users += 1;
    check_user_nick(login_msg.short_text, &num_users);
    send_success(login_msg.sender);
    send_all_subjects(login_msg.sender);
    send_all_users(login_msg.sender);
    printf("# New user [%s] no%d\n", Users[num_users-1].nick, num_users-1);
    send_new_user(Users[num_users-1].nick);
    // print_users(Users,&num_users); // print all users in logs always when new join
  }
  else{
    send_failure(login_msg.sender);
      printf("@ User no%d [%s] CAN'T login \n", user_index, login_msg.short_text);
  }
}


void add_subject(struct msg register_msg){
  if((check_subject(register_msg.short_text, &num_subjects) < 0) && (num_subjects < MAX_SUBJECTS)){
    strcpy(Subjects[num_subjects].name, register_msg.short_text);
    Subjects[num_subjects].number_of_subscribers = 0;
    num_subjects += 1;
    send_success(register_msg.sender);
    send_new_subject(register_msg.short_text);
    printf("# user [%s] create new subject [%s]\n", find_user_nick(Users, register_msg.sender, num_users), register_msg.short_text);
  }
  else{
    send_failure(register_msg.sender);
    printf("@ user [%s] CAN'T create new subject [%s] \n", find_user_nick(Users, register_msg.sender, num_users), register_msg.short_text);
  }
}


void join_subject(struct msg join_msg){
  int subject_index;
  if((subject_index = check_subject(join_msg.short_text, &num_subjects))>=0){
    int max_subers = Subjects[subject_index].number_of_subscribers;

    for(int i = 0; i <= max_subers; i++){
      if(strstr(find_user_nick(Users, join_msg.sender, num_users),Subjects[subject_index].subscribers[i].nick) != NULL){
        strcpy(Subjects[subject_index].subscribers[max_subers].nick, find_user_nick(Users, join_msg.sender, num_users));
        Subjects[subject_index].subscribers[max_subers].msg_counter = join_msg.number;
        Subjects[subject_index].number_of_subscribers += 1;
        send_success(join_msg.sender);
        printf("# user [%s] join to [%s]\n", find_user_nick(Users, join_msg.sender, num_users), join_msg.short_text);
        break;
      }
    }
  }
  else{
    send_failure(join_msg.sender);
    printf("@ user [%s] CAN'T join to [%s]\n", find_user_nick(Users, join_msg.sender, num_users), join_msg.short_text);
  }
  print_subjects(Subjects, &num_subjects);
}


void publish_msg(struct msg message){

    for(int i_subject = 0; i_subject < num_subjects; i_subject++){
      int sender_index, recipient_index;;

      if(strstr(Subjects[i_subject].name, message.short_text)){
        struct msg send_msg;
        strcpy(send_msg.long_text, message.long_text);
        strcpy(send_msg.short_text, message.short_text);
        send_msg.priority = message.priority;
        send_msg.type = MSG;
        send_msg.sender = message.sender;
        strcpy(send_msg.who, find_user_nick(Users, message.sender, num_users));

        sender_index = check_user_nick(find_user_nick(Users, message.sender, num_users), &num_users);

        for(int i_suber = 0; i_suber < Subjects[i_subject].number_of_subscribers; i_suber++){
          if(Subjects[i_subject].subscribers[i_suber].msg_counter != 0){
            Subjects[i_subject].subscribers[i_suber].msg_counter -= 1;
            int ipc_ID = find_user_ipc_ID(Users[recipient_index = check_user_nick(Subjects[i_subject].subscribers[i_suber].nick, &num_users)].pid ,&num_users);
            if (Users[recipient_index].muted[sender_index] != 1){
              printf("# sending msg to %s\n", Subjects[i_subject].subscribers[i_suber].nick);
              msgsnd(ipc_ID, &send_msg, sizeof(send_msg), 0);
            }
          }
        }
      }
      break;
    }
}


void mute_user(struct msg mute_msg){ //there can be added check if users exist;
  Users[check_user_nick(find_user_nick(Users, mute_msg.sender, num_users), &num_users)].muted[check_user_nick(mute_msg.short_text, &num_users)] = 1;
  send_success(mute_msg.sender);
}


int command_reader(){
  while(1){
    char input[32];
    scanf("%s", input);
    if(strstr(input, "/destroy") != NULL){
      int fd[2];
    	pipe(fd);
    	char buf;

    	if(fork() == 0){
    		close(fd[0]);
    		dup2(fd[1],1);
    		execlp("ipcrm", "ipcrm", "-a",NULL);
    		close(fd[1]);
    		exit(0);
    	}
        kill(getppid(), SIGKILL);
        return 0;
    }
  }
}

int main(){
  int ipc_in_ID = msgget(0x1337, 0600 | IPC_CREAT);

  int terminal_reader_pid;
  if((terminal_reader_pid = fork())==0){
    command_reader();
    return 0;
  }

while(1){
    fflush(stdout);

    for(int priority = 10; priority > 0; priority--){
      struct msg in_msg;
      if (msgrcv(ipc_in_ID, &in_msg, MSG_SIZE, priority, IPC_NOWAIT) > 0){

        if((find_user_ipc_ID(in_msg.sender, &num_users) < 0 )&&(in_msg.type != LOGIN)){
          send_failure(in_msg.sender);
        }
        else{
          switch (in_msg.type) {
            case LOGIN:
              add_user(in_msg);
              break;
            case REGISTER:
              add_subject(in_msg);
              break;
            case JOIN:
              join_subject(in_msg);
              break;
            case MSG:
              publish_msg(in_msg);
              break;
            case MUTE:
              mute_user(in_msg);
              break;
          }
        }
        priority = 10;
      }
    }
  }
return 0;
}
