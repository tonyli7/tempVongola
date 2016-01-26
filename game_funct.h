//--------ROLES----------
#define TOWNIE 0
#define MAFIASO 1
#define DOCTOR 2
#define COP 3

//-------STATUS----------
#define DEAD 0
#define ALIVE 1

//-------VOTE------------
#define NO -1
#define ABSTAIN 0
#define YES 1

//-------MARK------------
#define KILL 0
#define HEAL 1
#define NEUTRAL 2

typedef struct player{
  char name[256];
  char target[256];
  int role;
  int fd;
  int status;
  int vote;
  int mark;
}
  player;

void assign_roles(player* player_list);
int lynch_count(player* player_list);
int num_alive(player* player_list);
void shuffle(player* player_list);
int night_action(player* player_list);
int doctor_action(player* player_list);
int mafia_action(player* player_list);
int cop_action(player* player_list);
int player_index(char* name, player* player_list);
void print_DEAD(player* player_list);
void print_ALIVE(player* player_list);
char* get_role(int role);
