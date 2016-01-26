#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "game_funct.h"

void assign_roles(player* player_list){
  int roles[MAX_PLAYERS];

  int i;

  for(i = 0;  i < MAX_PLAYERS; i++){
    roles[i]=0;
  }
  roles[0]=1;
  roles[1]=1;

  for(i = 0; i < MAX_PLAYERS; i++){
    player_list[i].role=roles[i];
  }
}

int player_index(char* name, player* player_list){//find index of player
  int i;
 
  for(i = 0; i < MAX_PLAYERS; i++){   
    if (!strcmp(player_list[i].name,name)){      
      if (player_list[i].status == ALIVE){
	return i;
      }
    }
  } 
  return -1;
}

int doctor_action(player* player_list){

  int size=sizeof(player_list);
  int i=0;
  while (i<size){
    if (player_list[i].status == ALIVE){//if alive
     
      if (player_list[i].role == DOCTOR){//if doctor

	int target=player_index(player_list[i].target, player_list);//find index
	if (player_list[target].status == DEAD){//if target is dead
	  return -1;
	}
	player_list[target].mark=HEAL;
      }
     
    }
    i++;
  }
}

int mafia_action(player* player_list){
  int size=sizeof(player_list);
  int i=0;
  while (i<size){
  
   
    if (player_list[i].status == ALIVE){//if alive
      if (player_list[i].role == MAFIASO){//if mafiaso

	int target=player_index(player_list[i].target, player_list);
	if (player_list[target].status == DEAD){//if target is dead
	  return -1;
	}
	player_list[target].mark=KILL;

      }
    }
    i++;
  }
}

int cop_action(player* player_list){
  int size=sizeof(player_list);
  int i=0;
  while (i<size){
    if (player_list[i].status == ALIVE){
      if (player_list[i].role == COP){
	int target=player_index(player_list[i].target, player_list);
	if (player_list[target].status == DEAD){//if target is dead
	  return -1;
	}
	return player_list[target].role;//return target role
      }
    }
    i++;
  }
}

int night_action(player* player_list){
  int size=sizeof(player_list);
  int i=0;
 
  
  if (mafia_action(player_list) < 0){//do mafia stuff
    
    return -1;
  }
 
  if (doctor_action(player_list) < 0){//do doctor stuff
    
    return -1;
  }

  int revealed=cop_action(player_list);//do cop stuff
  if (revealed < 0){

    return -1;
  }
  printf("%s\n", get_role(revealed));

  while (i<size){
    if (player_list[i].mark == KILL){
      player_list[i].status=DEAD;
    }
    i++;
  }
  
}

char* get_role(int role){
  if (role == TOWNIE){
    return "Townie";

  }else if (role == MAFIASO){
    return "Mafiaso";

  }else if (role == DOCTOR){
    return "Doctor";

  }else if (role == COP){
    return "Cop";
  }
}

void print_DEAD(player* player_list){
  int size=sizeof(player_list);
  int i=0;

  printf("The Dead:\n\n");
  while (i<size){
    if (player_list[i].status == DEAD){
      printf("Name: %s ", player_list[i].name);
      
      printf("Role: %s\n", get_role(player_list[i].role));
    }
    i++;
  }
}

void print_ALIVE(player* player_list){
  int size=sizeof(player_list);
 
  int i=0;
  printf("The Living:\n\n");
  while(i<size){
    if (player_list[i].status == ALIVE){
      printf("Name: %s ", player_list[i].name);
      
      printf("Role: %s\n", get_role(player_list[i].role));
    }
    i++;
  }
}

int num_alive(player* player_list){
  int size=sizeof(player_list);
  int i=0;
  int alive=0;
  while (i<size){
    if (player_list[i].status == ALIVE){
      alive++;
    }
    i++;
  }
  return alive;
}

int lynch_count(player* player_list ){
  int size=sizeof(player_list);
  int i=0;
  int decision=0;
  while (i<size){
    if (player_list[i].vote == YES){
      decision++;
    }
    i++;
  }
  if (decision >= num_alive(player_list)/2+1){
    return 0;
  } 
  return -1;
}


/*player find_by_fd(int fd, player* player_list){
  int i;
  for (i = 0; i < size(player_list); i++)
    if(player_list[i].fd==fd)
      return player;
  return NULL;
}*/
int process_cmd(char* line,player p, player* player_list, int cycle){
  line  = trim(line);
  if(count_tokens(line)==1){
    if(strstr(line,"/p")==line){//print out player list
      //print_playerlist(player_list);
      return 1;
    }
    return 0;
  }
  else if(count_tokens(line)==2){
    char **ps = (char**)parse_by_space(line);
    if(strstr(ps[0],"/v")==line){
      player target=player_list[atoi(ps[1])];
	if(cycle%2==0){//if night
	  if(target.role==MAFIASO&&p.role==MAFIASO){
	    return 0;
	  }
	}
	if(p.mark){
	  player_list[p.mark].vote-=1;
	}
	target.vote+=1;
    }
    return 0;
  }
  else{
    return 0;
  }
}
/*
int main(){
  player* player_list=malloc(sizeof(player)*15);
 
  int i=0;
  while (i<8){
    char temp[256];
    sprintf(temp, "Player %d", i);//testing
    strcat(player_list[i].name, temp);
    player_list[i].status=ALIVE;
    player_list[i].mark=NEUTRAL;
    i++;
  }
 
  assign_roles(player_list);
  i=0;
 
  while (i<8){
    printf("%s: %d\n", player_list[i].name, player_list[i].role);//prints all player names and roles in array
    i++;
  }
 

  printf("\n\n");
  strcat(player_list[2].target, "Player 0");
  strcat(player_list[1].target, "Player 2"); 
  strcat(player_list[0].target, "Player 0");
  strcat(player_list[3].target, "Player 4");
  if (night_action(player_list) < 0){
    printf("Failed\n");
  }
  i=0;
 
  while (i<8){
    printf("%s: %d\n", player_list[i].name, player_list[i].status);
    i++;
  }
  printf("alive: %d\n", num_alive(player_list));
  print_ALIVE(player_list);
  print_DEAD(player_list);
}
*/
