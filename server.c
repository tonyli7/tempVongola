#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include <time.h>

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
  int day = 0;
  int hold = day;//helps to check if day has changed
  int num_players = 0;
  clock_t start,diff;
    
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
    //printf("Here\n");
    read_fds = master;
    if(num_players<1){
      num_players=0;
      for(i = 0; i < 15; i++)
	if(strlen(ulist[i])>0)
	  num_players++;
    }
    printf("Players:%d\n",num_players);
    if (num_players>=1){//once num_players has reached a number, game begins
      printf("HERE!\n");
      day = 1;
    }
    if(day>=1){
      if (hold!=day){
	printf("here2\n");
	start = clock();
      }
      else{
	printf("here!\n");
	diff = clock() - start;
	printf("%d\n",diff/CLOCKS_PER_SEC);
	if(diff/CLOCKS_PER_SEC>=10)
	  day++;
      }
    }
    printf("here3\n");
    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
      printf("select: %s\n", strerror(errno));
      exit(0);
    }
    if(hold!=day){
      printf("Here4\n");
      char d[20];
      if(day%2==1)
	sprintf(d,"Start of Day %d\n",day/2+1);
      else
	sprintf(d,"Start of Night %d\n",day/2);
      send_to_all(d,0,&master,fdmax,socket_id);
    }
    for(i = 3; i <= fdmax; i++){
      printf("Here2 %d,%d\n",hold,day);
      if(FD_ISSET(i, &read_fds)){
	if(i == socket_id){//if a client is trying to connect
	  //printf("Here3\n");
	    accept_client(&socket_id, &master, &fdmax);
	}else{
	  process(i, &master, fdmax, socket_id, ulist);
	}
	/*if(hold!=day){
	  printf("Here4\n");
	  char d[20];
	  if(day%2==1)
	    sprintf(d,"Start of Day %d",day/2+1);
	  else
	    sprintf(d,"Start of Night %d",day/2);
	    send(i,d,strlen(d),0);
	    }*/
      }
    }
    hold=day;
  }

  return 0;
}
