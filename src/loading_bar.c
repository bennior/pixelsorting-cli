#include "stdio.h"

#define TOTAL_STEPS 80 

const char todo = '.';
const char done = '#';
int current_step = 0;


void print_loading_bar(int total_amount, int current_amount) {
  
  if(current_amount >= total_amount)
    current_amount = total_amount;

  float percentage =  (float) current_amount / total_amount * 100;
  int target_step = (int)(((float) current_amount / total_amount) * TOTAL_STEPS);

  if(target_step > current_step)
    current_step = target_step;
  
  printf("\033[?25l"); //make cursor invisible
  printf("\033[97;101;1m"); //make background red, foreground white

  printf("\r"); //go to beginning of line


  printf("Progress: [ %0.1f%%]", percentage);


  printf("\033[97;100;1m"); //make background black, foreground white
  printf("\t[");

  for(int i = 0; i < current_step; i++)
    printf("%c", done);

  for(int i = current_step; i < TOTAL_STEPS; i++)
    printf("%c", todo);

  printf("]");
  printf("\033[0m"); //reset colors to default

  fflush(stdout);
}
