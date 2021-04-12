#include <stdio.h>

#define SIZE_SHORT_TEXT 16
#define SIZE_LONG_TEXT 256

enum msg_type {FAILURE, SUCCESS, LOGIN, REGISTER, JOIN, NEW_SUBJECT, NEW_USER, MSG, MUTE};


struct msg{
  long priority;
  int type;
  int sender;

  int number;
  char who[SIZE_SHORT_TEXT];
  char short_text[SIZE_SHORT_TEXT];
  char long_text[SIZE_LONG_TEXT];
};

long MSG_SIZE = sizeof(struct msg);


int check_priority(int priority){
  if (1 <= priority && priority <= 9){
    return priority;
  }
  else{
    return 9;
  }
}


void debug_msg(struct msg message){
  printf(" Priority: %ld \n",message.priority);
  printf(" Type: %d \n",message.type);
  printf(" Sender: %d \n",message.sender);
  printf(" Numebr: %d \n",message.number);
  printf(" Short text: %s \n",message.short_text);

}
