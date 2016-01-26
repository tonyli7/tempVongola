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

void send_to_all(char *line, int fd, fd_set *master, int fdmax, int socket_id){
  int i;
  for (i = 3; i <= fdmax; i++){
    if (FD_ISSET(i,master) && i != socket_id && i != fd){
      if(send(i, line, strlen(line), 0) == -1){
	printf("SEND: %s\n",strerror(errno));
      }
    }
  }
}

void process(int fd, fd_set *master, int fdmax, int socket_id, char** ulist){
  char buffer[256] = "\0";
  char line[256] = "\0";
  int num_bytes;
  num_bytes = recv(fd, buffer, sizeof(buffer), 0);
  buffer[num_bytes] = '\0';

  if(num_bytes <= 0){
    if(num_bytes == -1){
      printf("recv: %s\n", strerror(errno));
    }else if(num_bytes == 0){
      strcpy(line, ulist[fd-4]);
      strcat(line, " has left the town.\n");
    }
    close(fd);
    FD_CLR(fd, master);
    strcpy(ulist[fd-4], "\0");
  }else if(strlen(ulist[fd-4]) == 0){
    strcpy(ulist[fd-4], buffer);
    strcpy(line, ulist[fd-4]);
    strcat(line, " has entered the town.\n");
  }else{
    strcpy(line, ulist[fd-4]);
    strcat(line, ": ");
    strcat(line, buffer);
  }
  if(strlen(line) > 0){
    send_to_all(line, fd, master, fdmax, socket_id);
  }
}

void setup_socket(int *socket_id){
  //Create the socket
  *socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if(*socket_id == -1){
    printf("socket: %s\n", strerror(errno));
    exit(0);
  }

  int on = 1;
  
  //Bind to port/address
  struct sockaddr_in listener;
  listener.sin_family = AF_INET; //Socket type IPv4
  listener.sin_port = htons(56348); //Port #
  listener.sin_addr.s_addr = INADDR_ANY; //Bind to any incoming address

  if(setsockopt(*socket_id, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(*socket_id)) == -1){
    printf("setsockopt: %s\n", strerror(errno));
    exit(0);
  }

  if(bind(*socket_id, (struct sockaddr *)&listener, sizeof(listener)) == -1){
    printf("bind: %s\n", strerror(errno));
    exit(0);
  }

  if(listen(*socket_id, 15) == -1){
    printf("listen: %s\n", strerror(errno));
    exit(0);
  }
}

void accept_client(int *socket_id, fd_set *master, int *fdmax){
  struct sockaddr_in client_addr;
  socklen_t addrlen = sizeof(client_addr);
  int client_socket = accept(*socket_id, (struct sockaddr*)&client_addr, &addrlen);
  if(client_socket == -1){
    printf("accept: %s\n", strerror(errno));
  }else{
    FD_SET(client_socket, master);
    if(client_socket > 18){
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
    }
  }
}

int main() {
 

  int socket_id, i;
  fd_set master, read_fds;
  int cycle = 0;
  int hold = cycle;//helps to check if cycle has changed
  int num_players = 0;
  time_t start,diff;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
    
  char **ulist = (char**)calloc(15, sizeof(char *));
  for(i = 0; i < 15; i++){
    ulist[i] = (char *)calloc(16, sizeof(char));
  }
  setup_socket(&socket_id);
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(socket_id, &master);
  int fdmax = socket_id;
  while(1){
    read_fds = master;
    if(num_players < 2){
      num_players = 0;
      for(i = 0; i < 15; i++)
	if(strlen(ulist[i])>0)
	  num_players++;
    }
    if (num_players >= 2 && cycle == 0){//once num_players has reached a number, game begins
      cycle = 1;
      //lets go
      player* player_list=malloc(sizeof(player)*5);
      i=0;
      while (i<num_players){
	strcat(player_list[i].name, ulist[i]);
	player_list[i].status = ALIVE;
	i++;
      }
      assign_roles(player_list);
      print_ALIVE(player_list);
      
    }
    if(cycle >= 1){
      if (hold!=cycle){
	start = time(NULL);
	char d[20];
	if(cycle %2 == 1)
	  sprintf(d,"Start of Day %d\n", cycle/2+1);
	else
	  sprintf(d,"Start of Night %d\n", cycle/2);
	send_to_all(d, 0, &master, fdmax, socket_id);
	hold = cycle;
      }
      else{
	diff = time(NULL) - start;
	if(diff>=10){
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
	  accept_client(&socket_id, &master, &fdmax);
	}else{
	  process(i, &master, fdmax, socket_id, ulist);
	}
      }
    }
  }

  return 0;
}
