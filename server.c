#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include <time.h>

#include "game_funct.h"

void send_to_all(char *line, int fd, fd_set *master, int fdmax, player *player_list, int cycle){
  //printf("HERE9\n");
  if(fd != 0 && cycle > 0){
    int command = process_cmd(line, player_list[fd-4], player_list, cycle);
    if(command == -1){
      int x;
      sprintf(line,"Number of votes needed to Lynch: %d\n",num_alive(player_list)/2+1);
      for(x = 0; x < MAX_PLAYERS; x++){
	sprintf(line+strlen(line), "id: %d Name: %s Status: ", x, player_list[x].name);
	if(player_list[x].status==ALIVE)
	  sprintf(line+strlen(line), "ALIVE Votes: %d",player_list[x].vote);
	else
	  sprintf(line+strlen(line), "DEAD Role: %s\n",get_role(player_list[x].role));
	if(player_list[x].role==MAFIOSO&&player_list[fd-4].role==MAFIOSO&&player_list[x].status==ALIVE){
	  strcat(line," Role: MAFIOSO\n");
	}
	else
	  strcat(line,"\n");
	    
      }
      send(fd, line, strlen(line), 0);
      return;
    }
    else if(cycle==1){//no votes on Day 1
    }
    else if(command < MAX_PLAYERS){
      if(cycle%2 == 0){
	sprintf(line, "%s has voted to kill %s\n", player_list[fd-4].name, player_list[command].name);
      }else{
	sprintf(line,"%s has voted to lynch %s\n", player_list[fd-4].name, player_list[command].name);
      }
      if(player_list[fd-4].mark != -1)
	player_list[player_list[fd-4].mark].vote-=1;
      player_list[fd-4].mark=command;
      player_list[command].vote +=1;
      
      if(send(fd, line, strlen(line),0) ==-1)
	printf("SEND: %s\n",strerror(errno));
    }
  }
  int i;
  //printf("HERE10\n");
  for (i = 4; i <= fdmax; i++){
    if (FD_ISSET(i, master) && i != fd){
      //printf("HERE12\n");
      if((cycle && cycle%2 == 0 && player_list[fd-4].role == MAFIOSO && player_list[i-4].role == MAFIOSO) || cycle%2 == 1){
	//printf("HERE13\n");
	if(player_list[fd-4].status==ALIVE)
	  if(send(i, line, strlen(line), 0) == -1){
	    printf("SEND: %s\n", strerror(errno));
	  }
	//printf("AFTER CMD: %d\n",player_list[i-4].mark);
	//printf("HERE14\n");
      }
      else if(cycle==0){
	//printf("HERE11\n");
	if(send(i, line, strlen(line), 0) == -1){
	  printf("SEND: %s\n",strerror(errno));
	}
      }
    }
  }
} 

void process(int fd, fd_set *master, int fdmax, int socket_id, player *player_list, int *num_players, int cycle){
  char buffer[1024] = "";
  char line[1024] = "";
  int num_bytes;
  num_bytes = recv(fd, buffer, sizeof(buffer), 0);
  buffer[num_bytes] = '\0';
  
  if(num_bytes <= 0){
    if(num_bytes == -1){
      printf("recv: %s\n", strerror(errno));
    }else if(num_bytes == 0){
      sprintf(line, "%s has died.\n", player_list[fd-4].name);
      send_to_all(line, 0, master, fdmax, player_list, 0);
      line[0]='\0';
    }
    close(fd);
    FD_CLR(fd, master);
    player_list[fd - 4].status = DEAD;
  }else if(strlen(player_list[fd-4].name) == 0){
    strncpy(player_list[fd-4].name, buffer, 15);
    (*num_players)++;
    sprintf(line, "%s has entered the town.\n", player_list[fd-4].name);
    sprintf(line+strlen(line), "There are %d people in the town. We need %d to start.\n",*num_players,MAX_PLAYERS);
    send_to_all(line, 0, master, fdmax, player_list, 0);
    line[0]='\0';
  }else{
    sprintf(line, "%s: %s", player_list[fd-4].name, buffer);
  }
  if(strlen(line) > 0){
    send_to_all(line, fd, master, fdmax, player_list, cycle);
  }
}

void setup_socket(int *socket_id){
  //Create the socket
  *socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if(*socket_id == -1){
    printf("socket: %s\n", strerror(errno));
    exit(0);
  }

  //Bind to port/address
  struct sockaddr_in listener;
  listener.sin_family = AF_INET; //Socket type IPv4
  listener.sin_port = htons(56348); //Port #
  listener.sin_addr.s_addr = INADDR_ANY; //Bind to any incoming address

  int on = 1;
  if(setsockopt(*socket_id, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(*socket_id)) == -1){
    printf("setsockopt: %s\n", strerror(errno));
    exit(0);
  }

  if(bind(*socket_id, (struct sockaddr *)&listener, sizeof(listener)) == -1){
    printf("bind: %s\n", strerror(errno));
    exit(0);
  }

  if(listen(*socket_id, MAX_PLAYERS) == -1){
    printf("listen: %s\n", strerror(errno));
    exit(0);
  }
}

void accept_client(int socket_id, fd_set *master, int *fdmax, player *player_list,int cycle){
  struct sockaddr_in client_addr;
  socklen_t addrlen = sizeof(client_addr);
  int client_socket = accept(socket_id, (struct sockaddr*)&client_addr, &addrlen);
  if(client_socket == -1){
    printf("accept: %s\n", strerror(errno));
  }else{
    FD_SET(client_socket, master);
    if(client_socket > 3 + MAX_PLAYERS||cycle){
      char buffer[64] = "Sorry, too many players. Please wait.\n";
      if(send(client_socket, buffer, strlen(buffer), 0) == -1){
	printf("SEND: %s\n",strerror(errno));
      }
      close(client_socket);
      FD_CLR(client_socket, master);
    }else{
      if(client_socket > *fdmax){
	*fdmax = client_socket;
      }
      player_list[client_socket - 4].status = ALIVE;
      player_list[client_socket - 4].mark = -1;
    }
  }
}

int main(){
  int socket_id, i;
  fd_set master, read_fds;
  int cycle = 0;
  int hold = cycle;//helps to check if cycle has changed
  int num_players = 0;
  time_t start;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  player* player_list = calloc(MAX_PLAYERS, sizeof(player));
  setup_socket(&socket_id);
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(socket_id, &master);
  int fdmax = socket_id;

  while(1){
    read_fds = master;
    if (num_players == MAX_PLAYERS && cycle == 0){//once num_players has reached a number, game begins
      printf("HERE\n");
      cycle = 1;
      srand(time(NULL));
      assign_roles(player_list);
      for(i = 0; i < MAX_PLAYERS; i++){
	if(FD_ISSET(i+4, &master)){
	  char buffer[1024];
	  strcpy(buffer,"\nThe Game Has Started\n\nType in '/p' to view player list\nType in '/v' followed by a space and an id to vote.\n");
		  sprintf(buffer+strlen(buffer), "\nYou are a %s.\n", get_role(player_list[i].role));
	  if(send(i+4, buffer, strlen(buffer), 0) == -1){
	    printf("SEND: %s\n", strerror(errno));
	  }
	}
      }
      print_alive(player_list);
    }
    if(cycle >= 1){     
      if (hold != cycle){
	start = time(NULL);
	char d[1024]="";
	char deaths[1024]="";
	if(cycle > 1){
	  process_votes(player_list, cycle-1);
	  for(i = 0; i < MAX_PLAYERS; i++){
	    if(player_list[i].status == JUST_DEAD){
	      char *p = (char *)malloc(sizeof(char));
	      if(cycle%2 == 0)
		sprintf(p,"%s died by lynch\n",player_list[i].name);
	      else
		sprintf(p,"%s died by Mafia\n",player_list[i].name);
	      sprintf(p+strlen(p),"The person's role was %s\n",get_role(player_list[i].role));
	      strcat(deaths, p);
	      player_list[i].status=DEAD;
	    }
	  }
	  send_to_all(deaths, 0, &master, fdmax, player_list,0);
	  /*---------victory stuff--------------*/
	  if (victory(player_list) == TOWNIE){
	    char msg[]="\nTown wins!\n";
	   
	    send_to_all(msg, 0, &master, fdmax, player_list,0);
	    exit(0);
	  }else if (victory(player_list) == MAFIOSO){
	    char msg[]="\nMafia wins!\n";
	    send_to_all(msg, 0, &master, fdmax, player_list,0);
	  
	    exit(0);
	  }
	  //-------------------------------------
	}
	//printf("HERE6\n");
	if(cycle %2 == 1){
	  sprintf(d, "\nStart of Day %d\n", cycle/2+1);
	}else{
	  sprintf(d, "\nStart of Night %d\n", cycle/2);
	}
	//printf("HERE7\n");
	send_to_all(d, 0, &master, fdmax, player_list, 0);
	//printf("HERE8\n");
	for(i = 0; i < MAX_PLAYERS; i++){
	  player_list[i].vote=0;
	  player_list[i].mark=-1;
	}
	hold = cycle;
      }
      else{
	if(cycle == 1){
	  if(time(NULL) - start >= 10)
	    cycle++;
	}
        else if(cycle%2==1){
	  if(time(NULL) - start >= 45)
	    cycle++;
	}
	else{
	  if(time(NULL) - start >= 30)
	    cycle++;
	}
	    
	    
      }
    }
    if (select(fdmax+1, &read_fds, NULL, NULL, &timeout) == -1){
      printf("select: %s\n", strerror(errno));
      exit(0);
    }
    for(i = 3; i <= fdmax; i++){
      if(FD_ISSET(i, &read_fds)){
	if(i == socket_id){//if a client is trying to connect
	  accept_client(socket_id, &master, &fdmax, player_list, cycle);
	}else{
	  process(i, &master, fdmax, socket_id, player_list, &num_players, cycle);
	}
      }
    }
  }
  return 0;
}
