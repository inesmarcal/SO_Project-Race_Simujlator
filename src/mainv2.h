//Inês Martins Marçal 2019215917
//Noemia Gonçalves 2019219433
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <math.h>

#define FILENAME "config.txt"
#define MAXLINE 1024
#define MINTEAMS 3
#define PIPE_NAME "np_racesim_raceman"

pid_t pidMalfunction;

typedef struct {
  /* Message type */
  long carNum;
  /* Payload */
  int n; //MELHORAR este nome
} malfunctionMsg;

typedef struct
{
	int nlaps; //numero de voltas
	int num; //Número do carro
	int speed; //Velocidade
	int reliability; //Fiabilidade
	float fuel; //Combustivel
	char state; //Estado
  	float consumption; //Consumo
    int malfunction; //Avaria 
	int getFuel; //tempo do arranjo
  int numBoxStop; //numero de paragens na box
} car;

typedef struct
{
	char nameTeam[20]; //Nome da equipa
  int box; //Disponibilidade da box da equipa 0-disponivel -1-ocupada
  int carBoxIndex;
  int numSecurityMode; //Número de avarias no momento
  pthread_mutex_t boxMutex;
  pthread_mutex_t boxMutex2;
  pthread_cond_t boxCond;

} team;

typedef struct
{
  int time;
  int started;
  int numCars;
  int counterCar;
  int totalMalfunction;
  int totalRefill;
  int sigintPressed;
  int sigusr1Pressed;
  int controlCar;
  int winner;
  pthread_cond_t timeCond;
  pthread_cond_t restartTime;
  pthread_cond_t startedCond;
 
  pthread_cond_t endCond;
  pthread_mutex_t malfunctionMutex;
  pthread_mutex_t carMutex;
} race;

typedef struct
{
	int numtime;   // Número de unidades de tempo por segundo
	int dist;	   // Distância de cada volta
	int numLaps;   // Número de voltas
	int maxTeam;	//Número máximo de equipas da corrida
	int maxCarTeams; //Número maximo de carros por equipa
	int T_damage;  // Número de unidades de tempo entre novo cálculo de avaria
	int T_Box_min; //Tempo mínimo de reparação
	int T_Box_Max; //tempo máximo de reparação
	int capacity;  //Capacidade do depósito de combustível
	FILE *log;
}config;

pthread_t *thread;
int carshmid;
int fdNamedPipe;
pid_t original;
int (*pipes)[2];
int shmid;
int semid;
int mqid;
pthread_condattr_t cattr;
pthread_mutexattr_t mattr;
sem_t  *mutex, *logmutex, *stats, *box;
race *carRace;
config globalConfig;
team *teams;
car *cars;

//Permite saber o tempo atual de execução
struct tm currentTime()
{

	time_t s = time(NULL);
	struct tm current_time;
	localtime_r(&s, &current_time);
	return current_time;
}

void* carthreads(void*);
//Função que permite escrever no ficheiro "log.txt", mostrando o tempo atual de execução.
void writeLog(char *);

//Função que permite verificar e efetuar a leitura dos dados provenientes do ficheiro "config.txt"
int readFile( char *);

void checkCommand(char *, car *,char * );
void teammanager(int *);
void racemanager();
void sigint();
void malfunction();
void sigusr1();
void statistics();
void sigstp();
void cleanup();
void cleanSm();