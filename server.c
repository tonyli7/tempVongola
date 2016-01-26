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
    if (FD_ISSET(i, master) && i != socket_id && i != fd){
      if(send(i, line, strlen(line), 0) == -1){
	printf("SEND: %s\n", strerror(errno));
      }
    }
  }
}

void process(int fd, fd_set *master, int fdmax, int socket_id, player *player_list, int *num_players){
  char buffer[256] = "";
  char line[256] = "";
  int num_bytes;
  num_bytes = recv(fd, buffer, sizeof(buffer), 0);
  buffer[num_bytes] = '\0';

  if(num_bytes <= 0){
    if(num_bytes == -1){
      printf("recv: %s\n", strerror(errno));
    }else if(num_bytes == 0){
      sprintf(line, "%s has died.\n", player_list[fd-4].name);
    }
    close(fd);
    FD_CLR(fd, master);
    player_list[socket_id - 4].status = DEAD;
  }else if(strlen(player_list[fd-4].name) == 0){
    strncpy(player_list[fd-4].name, buffer, 15);
    player_list[fd-4].name[15] = '\0';
    printf("%s\n", player_list[fd-4].name);
    (*num_players)++;
    sprintf(line, "%s has entered the town.\n", player_list[fd-4].name);
  }else{
    sprintf(line, "%s: %s", player_list[fd-4].name, buffer);
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

  //Bind to port/address
  struct sockaddr_in listener;
  listener.sin_family = AF_INET; //Socket type IPv4
  listener.sin_port = htons(56349); //Port #
  listener.sin_addr.s_addr = INADDR_ANY; //Bind to any incoming address

  if(bind(*socket_id, (struct sockaddr *)&listener, sizeof(listener)) == -1){
    printf("bind: %s\n", strerror(errno));
    exit(0);
  }

  if(listen(*socket_id, MAX_PLAYERS) == -1){
    printf("listen: %s\n", strerror(errno));
    exit(0);
  }
}

void accept_client(int socket_id, fd_set *master, int *fdmax, player *player_list){
  struct sockaddr_in client_addr;
  socklen_t addrlen = sizeof(client_addr);
  int client_socket = accept(socket_id, (struct sockaddr*)&client_addr, &addrlen);
  if(client_socket == -1){
    printf("accept: %s\n", strerror(errno));
  }else{
    FD_SET(client_socket, master);
    if(client_socket > 3 + MAX_PLAYERS){
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

  player* player_list = calloc(MAX_PLAYERS, 86);
  setup_socket(&socket_id);
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(socket_id, &master);
  int fdmax = socket_id;

  while(1){
    read_fds = master;
    if (num_players == MAX_PLAYERS && cycle == 0){//once num_players has reached a number, game begins
      cycle = 1;
      assign_roles(player_list);
      print_ALIVE(player_list);
    }
    if(cycle >= 1){
      if (hold!=cycle){
	start = time(NULL);
	char d[20];
	if(cycle %2 == 1){
	  sprintf(d, "Start of Day %d\n", cycle/2+1);
	}else{
	  sprintf(d, "Start of Night %d\n", cycle/2);
	}
	send_to_all(d, 0, &master, fdmax, socket_id);
	hold = cycle;
      }
      else{
	if(time(NULL) - start >= 10){
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
	  accept_client(socket_id, &master, &fdmax, player_list);
	}else{
	  process(i, &master, fdmax, socket_id, player_list, &num_players);
	}
      }
    }
  }

  return 0;
}
