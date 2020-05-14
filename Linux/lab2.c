#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAX_COUNT 10
/*
char strings[10][30] = {{"\r\n1) First process"}, {"\r\n2) Second process"}, {"\r\n3) Third process"}, {"\r\n4) Fourth process"}, {"\r\n5) Fifth process"} , {"\r\n6) Sixth process"}, {"\r\n7) Seventh process"},{"\r\n8) Eighth process"}, {"\r\n9) Ninth process"}, {"\r\n10) Tenth process"}};
*/

char strings[10][30] = {{"1) First process"}, {"2) Second process"}, {"3) Third process"}, {"4) Fourth process"}, {"5) Fifth process"} , {"6) Sixth process"}, {"7) Seventh process"},{"8) Eighth process"}, {"9) Ninth process"}, {"10) Tenth process"}};

int printFlag = 0;
int flagEnd = 1;

struct sigaction printSignal, endSignal;

void canPrint(int signo)
{
  printFlag = 1;
}

void setEndFlag(int signo)
{
  flagEnd = 1;
}

int main(void)
{
  int currentNum = 0;
  int processNumber = 0;
  
  int flag = 0;
  
  initscr();
  clear();
  noecho();
  refresh();
  
  printSignal.sa_handler = canPrint;
  sigaction(SIGUSR1,&printSignal,NULL);
  
  endSignal.sa_handler = setEndFlag;
  sigaction(SIGUSR2,&endSignal,NULL);
  
  char c = 0;
  int i = 0;
  
  pid_t cpid[MAX_COUNT];
  
  
  while(c!='q')
  {
    c = getchar();
    
    switch(c)
    {
      case '+':	  
	if(processNumber < MAX_COUNT)
	{
	  cpid[processNumber] = fork();   // processNumber изначально 0
	  processNumber++;
	  
	  switch(cpid[processNumber-1])
	  {
	    case 0: // Дочерний процесс
	      flagEnd = 0;
	      while(!flagEnd)
	      {
		usleep(10000);  // Чтобы не загружать процессор
		if(printFlag)
		{
		  for(i=0; i<strlen(strings[processNumber-1]); i++)
		  {
		    if(flagEnd) 		
		      return 0;
		    
		    printf("%c",strings[processNumber-1][i]);
		    refresh();
		    usleep(70000);
		  }
		  refresh();
		  printFlag = 0;
		  kill(getppid(),SIGUSR2); // Сигнал родителю о завершении печати
		}
	      }
	      return 0;
	      break;
	      
	    case -1:
	      printf("Error!\n");
	      break;
	      
	    default:  // Родительский процесс
	      break;	    
	  }
	}
	break;
	
      case '-':
	if(processNumber > 0)
	{
	  kill(cpid[processNumber-1],SIGUSR2); // Завершаем последний дочерний процесс 
	  waitpid(cpid[processNumber-1],NULL,NULL); 

	  processNumber--;  
	  
	  if (currentNum >= processNumber)  // Если печатаемый в данный момент процесс завершился только что
	  {
	    currentNum = 0;	// Начинаем сначала
	    flag = 1;           // Флаг последнего по счёту процесса
	    
	    flagEnd = 1;	// Флаг окончания печати текущего процесса
	  }

	}	    
	    break;
    }
    
    if(flagEnd && processNumber>0)       // Если текущий процесс завершил печать
    {
      flagEnd = 0;
      if(currentNum >= processNumber - 1)    // Если номер текущего процесса последний,
	currentNum = 0;		       // то начинаем сначала   
      else if(!flag) currentNum++;    // иначе, если последний завершённый процесс не был последним по счёту, 
				      // переходим к следующему процессу      
      flag = 0;
      
      kill(cpid[currentNum],SIGUSR1); // Сигнал дочернему процессу о печати строки
    } 
    refresh();        
  }
  
  // Завершаем все дочерние процессы
  if(cpid[processNumber-1]!=0)
    for(;processNumber>=0;processNumber--)
    {
      kill(cpid[processNumber-1],SIGUSR2);
      waitpid(cpid[processNumber-1],NULL,NULL);
    }
  
  clear();  
  endwin();
  
  return 0;
}

