#include "stdio.h"

#include "loading_bar.h"

#define TOTAL_STEPS 80 
#define TODO '.'
#define DONE '#'

void print_loading_bar(int total_amount, int current_amount) {
  
  if(current_amount >= total_amount)
    current_amount = total_amount;

  float percentage =  (float) current_amount / total_amount * 100;
  int current_step = (int) ((float) current_amount / total_amount * TOTAL_STEPS);

#ifndef WINDOWS_OS
  printf("\033[?25l"); //make cursor invisible
  printf("\033[97;101;1m"); //make background red, foreground white
#endif
  printf("\r"); //go to beginning of line

  printf("Progress: [ %0.1f%%]", percentage);

#ifndef WINDOWS_OS
  printf("\033[97;100;1m"); //make background black, foreground white
#endif
  printf("\t["); //tab

  for(int i = 0; i < current_step; i++)
    printf("%c", DONE);

  for(int i = current_step; i < TOTAL_STEPS; i++)
    printf("%c", TODO);

  printf("]");
#ifndef WINDOWS_OS
  printf("\033[0m"); //reset colors to default
#endif

  fflush(stdout);
}
