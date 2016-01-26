
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "game_funct.h"

void shuffle(int *roles){
  int i;
  for(i = 0;i < MAX_PLAYERS; i++){
    int lucky = rand()%(MAX_PLAYERS-i) + i;
    int temp = roles[i];
    roles[i] = roles[lucky];
    roles[lucky] == temp;
  }
}

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

int doctor_action(player* player_list){
  int i;
  for(i = 0; i < MAX_PLAYERS; i++){
    if (player_list[i].status == ALIVE){//if alive
      if (player_list[i].role == DOCTOR){//if doctor
	int target = player_list[i].mark;
	if (player_list[target].status == DEAD){//if target is dead
	  return -1;
	}
      }
    }
  }
}

int mafia_action(player* player_list){
  int i;
  for(i = 0; i < MAX_PLAYERS; i++){
    if (player_list[i].status == ALIVE){//if alive
      if (player_list[i].role == MAFIOSO){//if mafioso
	int target = player_list[i].mark;
	if (player_list[target].status == DEAD){//if target is dead
	  return -1;
	}
      }
    }
  }
}

int cop_action(player* player_list){
  int i;
  for(i = 0; i < MAX_PLAYERS; i++){
    if (player_list[i].status == ALIVE){
      if (player_list[i].role == COP){
	int target = player_list[i].mark;
	if (player_list[target].status == DEAD){//if target is dead
	  return -1;
	}
	return player_list[target].role;//return target role
      }
    }
  }
}

int night_action(player* player_list){
  int i;  
  if(mafia_action(player_list) < 0){//do mafia stuff    
    return -1;
  } 
  if(doctor_action(player_list) < 0){//do doctor stuff
    return -1;
  }
  int revealed = cop_action(player_list);//do cop stuff
  if (revealed < 0){
    return -1;
  }
  printf("%s\n", get_role(revealed));
  /*
  for(i = 0; i < MAX_PLAYERS; i++){
    if (player_list[i].mark == KILL){
      player_list[i].status=DEAD;
    }
  }
  */
}

char* get_role(int role){
  if (role == TOWNIE){
    return "Townie";
  }else if (role == MAFIOSO){
    return "Mafioso";
  }else if (role == DOCTOR){
    return "Doctor";
  }else if (role == COP){
    return "Cop";
  }
}

void print_dead(player* player_list){
  int i;
  printf("The Dead:\n\n");
  for(i = 0; i < MAX_PLAYERS; i++){
    if (player_list[i].status == DEAD){
      printf("Name: %s ", player_list[i].name);
      printf("Role: %s\n", get_role(player_list[i].role));
    }
  }
}

void print_alive(player* player_list){
  int i;
  printf("The Living:\n\n");
  for(i = 0; i < MAX_PLAYERS; i++){
    if (player_list[i].status == ALIVE){
      printf("Name: %s ", player_list[i].name);      
      printf("Role: %s\n", get_role(player_list[i].role));
      //printf("Votes: %d\n", player_list[i].vote);
    }
  }
}

void print_players(player* player_list){
  int i;
  printf("People:\n");
  for(i = 0; i < MAX_PLAYERS; i++){
    printf("Name: %s ", player_list[i].name);
    if (player_list[i].status==ALIVE){
      printf("Status: ALIVE ");
      printf("Role: Not Revealed ");
      printf("Votes: %d\n", player_list[i].vote);
    }
    else{
      printf("Status: DEAD ");
      printf("Role: %s\n", get_role(player_list[i].role));
    }
  }
}
int num_alive(player* player_list){
  int i;
  int alive = 0;
  for(i = 0; i < MAX_PLAYERS; i++){
    if (player_list[i].status == ALIVE){
      alive++;
    }
  }
  return alive;
}

int process_cmd(char *line, player p, player *player_list, int cycle){
  char *command = malloc(256);
  strcpy(command, line);
  char *saveptr;
  strtok_r(command, " ", &saveptr);
  command = strtok_r(NULL, " ", &saveptr);
  char * next = strtok_r(NULL, " ", &saveptr);
  if(!strncmp("/p", command, 2)){
    return -1;
  }else if(!strcmp("/v", command)){
    char *endptr = (next+1);
    int target = strtol(next, &endptr, 10);
    if(target >= 0 || target < MAX_PLAYERS){
      if((cycle%2  == 0 && player_list[target].role != MAFIOSO && p.role == MAFIOSO) || cycle%2 == 1){
	printf("HERE\n");
	if(p.mark != -1){
	  player_list[p.mark].vote -= 1;
	}
	p.mark = target;
	printf("Mark,Target: %d, %d\n",p.mark,target);
	player_list[target].vote += 1;
	return target;
      }
    }
  }
  return MAX_PLAYERS;
}

void process_votes(player* player_list,int cycle){
  int i;
  int min = num_alive(player_list)/2+1;
  if(cycle%2==1){
    for(i = 0; i < MAX_PLAYERS; i++)
      if(player_list[i].vote >= min)
	player_list[i].status = JUST_DEAD;
  }else{
    for(i = 0; i < MAX_PLAYERS; i++)
      if(player_list[i].vote > 0)
	player_list[i].status = JUST_DEAD;
  }
}
