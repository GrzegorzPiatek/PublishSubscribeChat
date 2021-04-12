#ifndef SUBJECT_H
#define SUBJECT_H
#define MAX_SUBJECTS 32

struct subscriber{
  char nick[16];
  int msg_counter;
};

struct subject{
  char name[16];
  struct subscriber subscribers[16];
  int number_of_subscribers;
} Subjects[MAX_SUBJECTS];


int check_subject(const char *name, int *num_subjects){
  for(int i = 0; i <= *num_subjects; i++){
    if(strstr(Subjects[i].name, name) != NULL){
      return i;
    }
  }
    return -1;
}


void print_subjects(struct subject *Subjects, int *num_subjects){
  printf("Active subject(%d)\n", *num_subjects);
  for(int i = 0; i < *num_subjects; i++){
    printf("Name: %s, Number of subscribers: %d\n", Subjects[i].name, Subjects[i].number_of_subscribers);
    for(int j = 0; j < Subjects[i].number_of_subscribers; j++){
      printf("    %d, %s\n", j, Subjects[i].subscribers[j].nick);
    }
  }
}


#endif
