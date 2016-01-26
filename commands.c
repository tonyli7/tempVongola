#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "commands.h"

int count_tokens(char * line){
  int count = 1;
  while(*line){
    if (!strncmp(line," ",1)){
      count++;
      line+=1;
    }
    else
      line++;
  }
  return count;
}

char **parse_by_space(char * source){
  char *line = source;
  line = strsep(&line,"\n");
  char **ps=(char **)malloc((count_tokens(line)+1)*sizeof(char*));
  int i = 0;
  while(line){
    ps[i]=strsep(&line," ");
    i++;
  }
  return ps;
}

char * trim(char * line){
  if (line){
    while(*line==' ')
      line++;
    char * back = line + strlen(line)-1;
    while(*back==' '){
      back--;
    }
    *(back+1)='\0';
  }
  return line;
}

/*int main(){
  char str[20] = "Test This\n";
  char *s = str;
  printf("Num of Tokens: %d\n",count_tokens(s));

  char **list = parse_by_space(s);
  int i;
  for(i=0;list[i];i++)
    printf("%d: %s\n",i,list[i]);
}*/
