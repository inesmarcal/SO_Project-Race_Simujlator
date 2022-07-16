//Inês Martins Marçal 2019215917
//Noemia Gonçalves 2019219433
#define DEBUG
#include "race_Simulatorv2.c"
#define DEBUG


int main(int argc, char const *argv[])
{   //inicializar variaveis
	
	
	struct sigaction new_action, old_action;
	new_action.sa_handler = sigint; // sets the new handler
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;
	sigaction (SIGINT, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN) sigaction (SIGINT, &new_action, NULL);
	
	new_action.sa_handler = sigstp; // sets the new handler
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;
	sigaction (SIGTSTP, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN) sigaction (SIGTSTP, &new_action, NULL);

	original = getpid(); 

	//Semaforos
  	sem_unlink("LOGMUTEX");
	logmutex = sem_open("LOGMUTEX", O_CREAT | O_EXCL, 0700, 1);
	sem_unlink("MUTEX");
	mutex = sem_open("MUTEX", O_CREAT | O_EXCL, 0700, 1);
	sem_unlink("STATS");
	stats = sem_open("STATS", O_CREAT | O_EXCL, 0700, 0);
	sem_unlink("BOX");
	box = sem_open("BOX", O_CREAT | O_EXCL, 0700, 0);
	//ler configuracoes
    globalConfig.log = fopen("log.txt", "w+");
    readFile(FILENAME);

	//escrever para o log
	writeLog("Simulador Starting\n");

    //memoria partilhada
    if ((shmid = shmget(IPC_PRIVATE, sizeof(race) + sizeof(team) * globalConfig.maxTeam + sizeof(car) * globalConfig.maxTeam * globalConfig.maxCarTeams, IPC_CREAT | 0700)) < 0)
	{
		perror("Error in shmget with IPC_CREAT\n");
		exit(1);
	}
	if ((carRace = (race *)shmat(shmid, NULL, 0)) == (race *)-1)
	{
		perror("Shmat error!");
		exit(1);
	}

		teams = (team *)(carRace + 1);
		cars = (car * ) (teams + globalConfig.maxTeam);
		carRace->time = 0;
		carRace->started = 0;
		carRace->numCars=0;
		carRace->counterCar=0;
		carRace->sigintPressed = 0;
		carRace->sigusr1Pressed = 0; 
		carRace->winner = 0;
		pthread_mutexattr_setpshared(&mattr,PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&carRace->carMutex,&mattr);
		pthread_mutex_init(&carRace->malfunctionMutex,&mattr);
		
		pthread_condattr_setpshared(&cattr,PTHREAD_PROCESS_SHARED);
		pthread_cond_init(&carRace->startedCond,&cattr);
		pthread_cond_init(&carRace->timeCond,&cattr);
		pthread_cond_init(&carRace->restartTime,&cattr);
		
		pthread_cond_init(&carRace->endCond,&cattr);
		
    //Inicializar nomes das equipas para posterior controlo
    for (int i = 0; i < globalConfig.maxTeam;i++){
		strcpy((teams + i)->nameTeam,"\0");
		pthread_mutex_init(&(teams+i)->boxMutex,&mattr);
		pthread_mutex_init(&(teams+i)->boxMutex2,&mattr);
		pthread_cond_init(&(teams+i)->boxCond,&cattr);
		
	}


    //Inicializar numero dos carros para posterior controlo
    for (int i = 0; i < globalConfig.maxCarTeams*globalConfig.maxTeam;i++){
		(cars+i)->num = -1;
    }

	

	//criar named pipe
	if ((mkfifo(PIPE_NAME, O_CREAT | O_EXCL | 0600) < 0) && (errno != EEXIST))
	{
		perror("Erro ao criar pipe: ");
		exit(1);
	}

	if ((mqid = msgget(IPC_PRIVATE, IPC_CREAT|0777)) < 0){
  	printf("Erro na criação da Message Queue.\n");
  	exit(1);
  }
	fflush(globalConfig.log);
	
	//thread tempo 
	pthread_t timer;
	pthread_create(&timer,NULL,timeCounter,NULL);

	//iniciar simulatorManager
	if ((fork()) == 0){
		signal(SIGINT,SIG_IGN);
		signal(SIGTSTP,SIG_IGN);
		racemanager();
		exit(0);
	}


	//iniciar malfunctionManager
	if ((fork()) == 0){
		signal(SIGTSTP,SIG_IGN);
		signal(SIGINT,SIG_IGN);
		malfunctionManager();
		exit(0);
	}

	

	//abrir named pipe para escrita
 	int fd;
 	if ((fd = open(PIPE_NAME,O_RDWR)) < 0){
 		perror("Erro ao abrir pipe:");
 		exit(1);
 	}

	//espera pelo simulatorManager e pelo malfunctionManager
	 wait(NULL);
	 wait(NULL);
	

	writeLog("Simulador Closing\n");
	cleanup();

	
	return 0;
}
