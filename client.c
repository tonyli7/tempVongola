#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>


void get_name(char *buffer, int socket_id){
  while(strlen(buffer) == 0){//if the client doesn't have a name, have the next input be the name.
    printf("Please enter your desired name: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strlen(buffer) - 1] = '\0';
  }
}

void process(int fd, int sockfd, char *name){//controls sending and receiving
  char buffer[256];
  int num_bytes;
  if (fd == 0){
    //send from stdin to server
    fgets(buffer, sizeof(buffer), stdin);
    send(sockfd, buffer, strlen(buffer), 0);
  }else{
    //Receive from server
    num_bytes = recv(sockfd, buffer, sizeof(buffer),0);
    if(num_bytes == -1){//if error in receiving
      printf("recv: %s\n",strerror(errno));
      exit(0);
    }else if(num_bytes == 0){//if socket has closed
      printf("Server closed\n");
      exit(0);
    }else{//receive stuff
      if(strlen(name) == 0){//send name to server if not created
	get_name(name, sockfd);
	send(sockfd, name, strlen(name), 0);
      }
    }
      buffer[num_bytes]='\0';
      printf("%s", buffer);//print the message received out
      fflush(stdout);
  }
}

void setup_socket(int *socket_id){
  //Create the socket
  *socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if(*socket_id == -1){
    printf("socket: %s\n", strerror(errno));
    exit(0);
  }

  struct sockaddr_in sock;
  sock.sin_family = AF_INET;
  sock.sin_port = htons(56348);
  //Set the IP address to connect to
  //127.0.0.1 is the "loopback" address of any machine
  inet_aton("127.0.0.1", &(sock.sin_addr));
  if(connect(*socket_id, (struct sockaddr *)&sock, sizeof(sock)) == -1){
    printf("connect: %s\n", strerror(errno));
    exit(0);
  }
}

int main(int argc, char **argv) {

  int socket_id;
  char name[256] = "\0";
  fd_set master;
  fd_set read_fds;

  setup_socket(&socket_id);
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(0, &master);
  FD_SET(socket_id, &master);
  
  int fdmax = socket_id;

  get_name(name, socket_id);
  send(socket_id, name, strlen(name), 0);
  printf("Welcome to Vongola, a modified Mafia in C!\n");


  while(1){
    read_fds = master;
    if (select(fdmax+1, &read_fds, NULL, NULL, NULL)==-1){//if there is activity
      printf("select: %s\n", strerror(errno));
      exit(0);
    }
    if(FD_ISSET(0, &read_fds)){//for sending
      process(0, socket_id, name);
    }
    if(FD_ISSET(socket_id, &read_fds)){//for receiving
      process(socket_id, socket_id, name);
    }
  }
}
