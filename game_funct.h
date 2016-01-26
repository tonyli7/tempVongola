#ifndef TOWNIE
#define MAX_PLAYERS 2

//--------ROLES----------
#define TOWNIE 0
#define MAFIOSO 1
#define DOCTOR 2
#define COP 3

//-------STATUS----------
#define DEAD 0
#define ALIVE 1
#define JUST_DEAD 2

typedef struct player{
  char name[16];
  int role;
  int status;
  int vote;
  int mark;
}
  player;

void shuffle(int *roles);
void assign_roles(player* player_list);
int lynch_count(player* player_list);
int num_alive(player* player_list);
void shuffle(int* list);
int night_action(player* player_list);
int doctor_action(player* player_list);
int mafia_action(player* player_list);
int cop_action(player* player_list);
int night_action(player* player_list);
char* get_role(int role);
void print_dead(player* player_list);
void print_alive(player* player_list);
int num_alive(player* player_list);
int lynch_count(player* player_list);
int process_cmd(char *line, player p, player *player_list, int cycle);

#endif
