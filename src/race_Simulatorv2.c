//Inês Martins Marçal 2019215917
//Noemia Gonçalves 2019219433
#include "mainv2.h"
#define DEBUG

void cleanup()
{
	unlink(PIPE_NAME);
	close(fdNamedPipe);

	pthread_cond_destroy(&carRace->startedCond);
	pthread_cond_destroy(&carRace->restartTime);
	pthread_cond_destroy(&carRace->timeCond);
	pthread_cond_destroy(&carRace->endCond);

	pthread_mutex_destroy(&carRace->carMutex);
	pthread_mutex_destroy(&carRace->malfunctionMutex);

	pthread_mutexattr_destroy(&mattr);
	pthread_condattr_destroy(&cattr);

	for (int i = 0; i < globalConfig.maxTeam ; i++){
			pthread_cond_destroy(&(teams + i)->boxCond);
			pthread_mutex_destroy(&(teams + i)->boxMutex);
			pthread_mutex_destroy(&(teams + i)->boxMutex2);
			
	}
	msgctl(mqid, IPC_RMID, NULL);
	shmctl(shmid, IPC_RMID, NULL);

	sem_close(stats);
	sem_unlink("STATS");
	sem_close(mutex);
	sem_unlink("MUTEX");
	sem_close(logmutex);
	sem_unlink("LOGMUTEX");
	sem_close(box);
	sem_unlink("BOX");
	fclose(globalConfig.log);
}


void cleanSm()
{
	//limpar dados da corrida dos carros
	for (int i = 0; i < globalConfig.maxCarTeams * globalConfig.maxTeam; i++)
	{
		(cars + i)->malfunction = 0;
		(cars + i)->nlaps = 0;
		(cars + i)->state = 0;
	}

	//limpar dados equipas dos carros
	for (int i = 0; i < globalConfig.maxTeam; i++)
	{
		(teams + i)->box = 0;
		(teams + i)->carBoxIndex = 0;
		(teams + i)->numSecurityMode = 0;
	}

	//limpar contadores
	carRace->sigintPressed = 0;
	carRace->counterCar = 0;
	carRace->time = 0;
	carRace->totalMalfunction = 0;
	carRace->totalRefill = 0;
	carRace->winner = 0;
}
void writeLog(char *s)
{
	char buffer[200];
	struct tm time = currentTime();
	sprintf(buffer, "%02d:%02d:%02d %s", time.tm_hour, time.tm_min, time.tm_sec, s);
	sem_wait(logmutex);
	printf("%s", buffer);
	fwrite(buffer, sizeof(char), strlen(buffer), globalConfig.log);
	sem_post(logmutex);
}

int readFile(char *filename)
{
	char *token;
	FILE *fconfig;
	char buffer[MAXLINE];
	const char *delim = ",\n ";
	char *saveptr, *ptr;
	int num = 0;

	if ((fconfig = fopen(filename, "r")) == NULL)
	{
		writeLog("CANNOT OPEN FILE\n");
		return -1;
	}

	fgets(buffer, MAXLINE, fconfig); //line 1
	token = strtok_r(buffer, delim, &saveptr);
	num = strtol(token, &ptr, 10);

	if (num != 0 && strtok_r(NULL, delim, &saveptr) == NULL && strlen(ptr) == 0)
	{
		globalConfig.numtime = (int)num;
	}
	else
	{
		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}

	fgets(buffer, MAXLINE, fconfig); //line 2
	token = strtok_r(buffer, delim, &saveptr);
	num = strtol(token, &ptr, 10);
	if (num != 0 && strlen(ptr) == 0)
	{
		globalConfig.dist = (int)num;
	}
	else
	{
		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}
	token = strtok_r(NULL, delim, &saveptr);
	num = strtol(token, &ptr, 10);
	if (num != 0 && strtok_r(NULL, delim, &saveptr) == NULL && strlen(ptr) == 0)
	{
		globalConfig.numLaps = (int)num;
	}
	else
	{
		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}

	fgets(buffer, MAXLINE, fconfig); //line 3
	token = strtok_r(buffer, delim, &saveptr);

	num = strtol(token, &ptr, 10);
	if (num >= 3 && strtok_r(NULL, delim, &saveptr) == NULL && strlen(ptr) == 0)
	{
		globalConfig.maxTeam = (int)num;
	}
	else
	{

		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}

	fgets(buffer, MAXLINE, fconfig); //line 4
	token = strtok_r(buffer, delim, &saveptr);
	num = strtol(token, &ptr, 10);
	if (num != 0 && strtok_r(NULL, delim, &saveptr) == NULL && strlen(ptr) == 0)
	{
		globalConfig.maxCarTeams = (int)num;
	}
	else
	{

		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}

	fgets(buffer, MAXLINE, fconfig); //line 5
	token = strtok_r(buffer, delim, &saveptr);
	num = strtol(token, &ptr, 10);
	if (num != 0 && strtok_r(NULL, delim, &saveptr) == NULL && strlen(ptr) == 0)
	{
		globalConfig.T_damage = (int)num;
	}
	else
	{
		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}

	fgets(buffer, MAXLINE, fconfig); //line 6
	token = strtok_r(buffer, delim, &saveptr);
	num = strtol(token, &ptr, 10);
	if (num != 0 && strlen(ptr) == 0)
	{
		globalConfig.T_Box_min = (int)num;
	}
	else
	{
		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}

	token = strtok_r(NULL, delim, &saveptr);
	num = strtol(token, &ptr, 10);
	if (num != 0 && strtok_r(NULL, delim, &saveptr) == NULL && strlen(ptr) == 0)
	{
		globalConfig.T_Box_Max = (int)num;
	}
	else
	{
		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}

	fgets(buffer, MAXLINE, fconfig); //line 7
	token = strtok_r(buffer, delim, &saveptr);
	num = strtol(token, &ptr, 10);
	if (num != 0 && strtok_r(NULL, delim, &saveptr) == NULL && strlen(ptr) == 0)
	{
		globalConfig.capacity = (int)num;
	}
	else
	{
		writeLog("INVALID CONFIGURATION FILE\n");
		return -1;
	}
	while (fgets(buffer, MAXLINE, fconfig))
	{
		if (strlen(buffer) != 1)
		{
			writeLog("INVALID CONFIGURATION FILE\n");
			return -1;
		}
	}
	return 0;
}

void checkCommand(char *command, car *c, char *nameTeam)
{
	char token[MAXLINE] = "\0", arg[MAXLINE] = "\0", *saveptr, *ptr;
	c->num = -1;
	const char *delim = "\n, ";
	long num;
	float num_float;
	char *t;
	strcpy(token, command);

	//printf("TOKEN: %s\n",token);
	if (strstr(token, "ADDCAR TEAM:") != token)
	{
		return;
	}
	strcpy(token, (token + strlen("ADDCAR TEAM:")));
	strcpy(arg, token);

	t = strtok_r(token, delim, &saveptr);
	if (!t)
	{
		c->num = -1;
		return;
	}
	//copiar a string para o carro
	strcpy(nameTeam, t);
	strcat(nameTeam, "\0");

	//mover arg para CAR
	strcpy(arg, arg + strlen(token) + 2);
	strcpy(token, arg);

	//verficar se o proximo parametro e o carro
	if (strstr(token, "CAR:") != token)
	{
		return;
	}
	//avancar para o numero
	strcpy(token, (token + strlen("CAR:")));
	strcpy(arg, token);

	t = strtok_r(token, delim, &saveptr);
	if (!t)
	{
		c->num = -1;
		return;
	}
	//conversao e verificacao
	num = strtol(t, &ptr, 10);
	if (num == 0 || strlen(ptr) > 0)
	{
		return;
	}
	c->num = (int)num;

	//avancar para SPEED
	strcpy(arg, arg + strlen(token) + 2);
	strcpy(token, arg);

	if (strstr(token, "SPEED:") != token)
	{
		c->num = -1;
		return;
	}

	//avancar para o valor
	strcpy(token, (token + strlen("SPEED:")));
	strcpy(arg, token);

	t = strtok_r(token, delim, &saveptr);
	if (!t)
	{
		c->num = -1;
		return;
	}
	//conversao e verificacao
	num = strtol(t, &ptr, 10);
	if (num == 0 || strlen(ptr) > 0)
	{
		c->num = -1;
		return;
	}
	c->speed = (int)num;

	//avanca para consumption
	strcpy(arg, arg + strlen(token) + 2);
	strcpy(token, arg);
	if (strstr(token, "CONSUMPTION:") != token)
	{

		c->num = -1;
		return;
	}

	//avancar para o valor
	strcpy(token, (token + strlen("CONSUMPTION:")));
	strcpy(arg, token);
	t = strtok_r(token, delim, &saveptr);
	if (!t)
	{
		c->num = -1;
		return;
	}
	//verificar float da consumption
	num_float = strtof(t, &ptr);
	if (num_float == 0 || strlen(ptr) > 0)
	{
		c->num = -1;
		return;
	}

	c->consumption = num_float;
	//avancar para reliability
	strcpy(arg, arg + strlen(token) + 2);
	strcpy(token, arg);

	if (strstr(token, "RELIABILITY:") != token)
	{

		c->num = -1;
		return;
	}
	//avancar para o valor
	strcpy(token, (token + strlen("RELIABILITY:")));
	strcpy(arg, token);
	t = strtok_r(token, delim, &saveptr);
	if (!t)
	{
		c->num = -1;
		return;
	}
	num = strtol(t, &ptr, 10);
	if (num == 0 || strlen(ptr) > 0)
	{

		c->num = -1;
		return;
	}
	c->reliability = (int)num;
}

void racemanager()
{
	printf("Race Manager com pid %d\n", getpid());
#ifdef DEBUG
	printf("Race Manager criado\n");
#endif

	//variaveis

	int fd[globalConfig.maxTeam][2];
	pipes = fd;
	int fdmax = 0;
	int numTeam, addedCar, i, j, excl = 0;
	fd_set readSet;
	char command[MAXLINE];
	char nameTeam[20];
	char logMessage[2 * MAXLINE];
	car *pcar = (car *)malloc(sizeof(car));
	int nread;
	pid_t teamsPid[globalConfig.maxTeam];
	

	//criar teamManagers
	for (i = 0; i < globalConfig.maxTeam; i++)
	{
		pipe(fd[i]);
		if ((teamsPid[i] = fork()) == 0)
		{
			teammanager(&i);
		}
		if (fdmax < fd[i][1])
			fdmax = fd[i][1];

		close(fd[i][1]);
	}
	
	signal(SIGUSR1,sigusr1);
	//abrir named pipe para leitura
	if ((fdNamedPipe = open(PIPE_NAME, O_RDWR)) < 0)
	{
		perror("Cannot open pipe for reading");
		exit(1);
	}

	numTeam = 0;
	while (1)
	{
		FD_ZERO(&readSet);
		for (i = 0; i < globalConfig.maxTeam; i++)
		{
			FD_SET(fd[i][0], &readSet);
			if (fdmax < fd[i][0])
				fdmax = fd[i][0];
		}
		FD_SET(fdNamedPipe, &readSet);
		if (fdmax < fdNamedPipe)
			fdmax = fdNamedPipe;

		if ((select(fdmax + 1, &readSet, NULL, NULL, NULL)) > 0)
		{
			if (FD_ISSET(fdNamedPipe, &readSet))
			{
				nread = read(fdNamedPipe, &command, sizeof(command));
				command[nread - 1] = '\0';
				if (strcmp(command, "START RACE!") == 0)
				{

					if (numTeam == globalConfig.maxTeam)
					{
						if (carRace->started == 0)
						{
							sem_wait(mutex);
							carRace->started = 1;
							sem_post(mutex);
							pthread_cond_broadcast(&carRace->startedCond);
							writeLog("RACE STARTING\n");
						}
						else
						{
							writeLog("RACE HAS ALREADY STARTED\n");
						}
					}
					else
					{
						writeLog("CANNOT START, NOT ENOUGH TEAMS\n");
					}
				}
				else
				{

					pcar->num = -1;
					checkCommand(command, pcar, nameTeam);
					if (pcar->num == -1)
					{
						sprintf(logMessage, "WRONG COMMAND => %s\n", command);
						writeLog(logMessage);
					}
					else
					{
						excl = 0;
						addedCar = 0;
						if (pcar->num < 1)
						{
							excl = 1;
							sprintf(logMessage, "CAR NUMBER MUST BE POSITIVE\n");
							writeLog(logMessage);
						}
						else
						{
							if (carRace->started == 1)
							{
								excl = 1;
								sprintf(logMessage, "REJECTED, RACE ALREADY STARTED!\n");
								writeLog(logMessage);
							}
							else
							{
								for (i = 0; i < globalConfig.maxTeam * globalConfig.maxCarTeams; i++)
								{
									if (pcar->num == (cars + i)->num)
									{
										sprintf(logMessage, "ALREADY EXISTS A CAR WITH NUMBER %d\n", pcar->num);
										writeLog(logMessage);
										excl = 1;
									}
								}
							}
						}
						if (excl == 0)
						{
							for (i = 0; i < globalConfig.maxTeam; i++)
							{
								//verifica se a equipa ja existe

								if (strcmp((teams + i)->nameTeam, nameTeam) == 0)
								{

									for (int j = i * globalConfig.maxCarTeams; j < i * globalConfig.maxCarTeams + globalConfig.maxCarTeams; j++)
									{
										if ((cars + j)->num == -1)
										{

											sem_wait(mutex);
											(cars + j)->num = pcar->num;
											(cars + j)->speed = pcar->speed;
											(cars + j)->nlaps = 0;
											(cars + j)->fuel = globalConfig.capacity;
											(cars + j)->state = 0;
											(cars + j)->malfunction = 1;
											(cars + j)->getFuel = 0;
											(cars + j)->consumption = pcar->consumption;
											(cars + j)->reliability = pcar->reliability;
											carRace->controlCar++;
											sem_post(mutex);
											sprintf(logMessage, "NEW CAR LOADED => TEAM: %s CAR: %d SPEED: %d CONSUMPTION: %f RELIABILITY: %d\n", nameTeam, pcar->num, pcar->speed, pcar->consumption, pcar->reliability);
											writeLog(logMessage);
											addedCar = 1;
											break;
										}
									}
									if (addedCar == 0)
									{
										sprintf(logMessage, "TEAM IS FULL COMMAND REJECTED");
										writeLog(logMessage);
									}
									break;
								}
								else
								{
									//verifica se existe espaco para criar uma nova equipa
									if (strlen((teams + i)->nameTeam) == 0)
									{
										sem_wait(mutex);
										strcpy((teams + i)->nameTeam, nameTeam);
										j = i * globalConfig.maxCarTeams;
										(cars + j)->num = pcar->num;
										(cars + j)->speed = pcar->speed;
										(cars + j)->nlaps = 0;
										(cars + j)->state = 0;
										(cars + j)->getFuel = 0;
										(cars + j)->malfunction = 0;
										(cars + j)->fuel = globalConfig.capacity;
										(cars + j)->consumption = pcar->consumption;
										(cars + j)->reliability = pcar->reliability;
										carRace->controlCar++;
										sem_post(mutex);
										sprintf(logMessage, "NEW CAR LOADED => TEAM: %s, CAR: %d, SPEED: %d, CONSUMPTION: %f, RELIABILITY: %d\n", nameTeam, pcar->num, pcar->speed, pcar->consumption, pcar->reliability);
										writeLog(logMessage);
										numTeam++;
										addedCar = 1;
										break;
									}
								}
							}
							if (addedCar == 0)
							{
								sprintf(logMessage, "MAX TEAMS REACHED\n");
								writeLog(logMessage);
							}
						}
					}
				}
			}

			for (i = 0; i < globalConfig.maxTeam; i++)
			{
				if (FD_ISSET(fd[i][0], &readSet))
				{
					nread = read(fd[i][0], &command, sizeof(command));
					command[nread - 1] = '\0';
					writeLog(command);

				
				}
			}
				if (carRace->controlCar == 0)
					{
						pthread_cond_broadcast(&carRace->endCond);
						for (int k = 0; k < globalConfig.maxTeam; k++)
						{
							pthread_cond_signal(&(teams + k)->boxCond);
						}
						pthread_cond_signal(&carRace->restartTime);
						pthread_cond_signal(&carRace->timeCond);
						pthread_cond_signal(&carRace->startedCond);
						sem_post(stats);
						exit(0);
					
					}
		}
	}

}

void teammanager(int *teamIndex)
{

	for (int i = 0; i < globalConfig.maxCarTeams; i++)
	{
		if (i > *teamIndex)
			break;
		close(pipes[i][0]);
	}
	(teams + *teamIndex)->carBoxIndex = -1;
	pthread_mutex_lock(&carRace->carMutex);
	while (carRace->started == 0)
	{
		pthread_cond_wait(&carRace->startedCond, &carRace->carMutex);
	}
	pthread_mutex_unlock(&carRace->carMutex);
	pthread_t idThreads[globalConfig.maxCarTeams];
	int index[globalConfig.maxCarTeams][2];
	int countCar = 0;
	for (int i = 0; i < globalConfig.maxCarTeams; i++)
	{
		if ((cars + (*teamIndex * globalConfig.maxCarTeams + i))->num != -1)
		{
			index[i][0] = *teamIndex;
			index[i][1] = countCar  + *teamIndex * globalConfig.maxCarTeams;
			sem_wait(mutex);
			carRace->numCars++;
			sem_post(mutex);
			pthread_create(&idThreads[i], NULL, carthreads, (void *)&index[i]);
			countCar++;
		}
	}
	int carId;
	int trepair = 0;
	int timeEntry = 0;
	while (carRace->controlCar > 0)
	{
		pthread_mutex_lock(&(teams + *teamIndex)->boxMutex);
		while ((teams + *teamIndex)->carBoxIndex == -1 && carRace->controlCar > 0)
		{
			pthread_cond_wait(&(teams + *teamIndex)->boxCond, &(teams + *teamIndex)->boxMutex);
		}
		pthread_mutex_unlock(&(teams + *teamIndex)->boxMutex);
		if( carRace->controlCar == 0 ){
	
				break;
		
		}
		carId = (teams + *teamIndex)->carBoxIndex;
		if ((cars + carId)->state == 1)
		{
		
			(teams + *teamIndex)->box = 1;
			(teams + *teamIndex)->carBoxIndex = -1;
			continue;
		}
		else
		{
			(teams + *teamIndex)->carBoxIndex = -1;
			(teams + *teamIndex)->box = 2;
			sem_post(box);
			if ((cars + carId)->malfunction == 1) // avaria
			{

				srand(getpid());
				trepair = (rand() % (globalConfig.T_Box_Max - globalConfig.T_Box_min + 1)) + globalConfig.T_Box_min;
			}
			if ((cars + carId)->getFuel == 1)
			{
				trepair += 2;
				carRace->totalRefill++;
			}
			timeEntry = carRace->time;
			carRace->numCars--;
			pthread_cond_signal(&carRace->restartTime);

			pthread_mutex_lock(&(teams + *teamIndex)->boxMutex);
			while (carRace->time < timeEntry + trepair && carRace->sigintPressed != 1 )
			{
				pthread_cond_wait(&carRace->timeCond, &(teams + *teamIndex)->boxMutex);
			}
			pthread_mutex_unlock(&(teams + *teamIndex)->boxMutex);
			carRace->numCars++;
			pthread_cond_signal(&carRace->restartTime);
			if ((cars + carId)->state == 1)
			{
				(teams + *teamIndex)->numSecurityMode--;
			}
			if ((teams + *teamIndex)->numSecurityMode == 0)
			{
				(teams + *teamIndex)->box = 0;
			}
			else
			{
				(teams + *teamIndex)->box = 1;
			}
			(teams + *teamIndex)->carBoxIndex = -1;
			if ((cars + carId)->getFuel == 1)
			{
				(cars + carId)->getFuel = 0;
				(cars + carId)->fuel = globalConfig.capacity;
			}
			(cars + carId)->malfunction = 0;
			pthread_cond_signal(&(teams + *teamIndex)->boxCond);
			(teams + *teamIndex)->carBoxIndex = -1;
		}
	}

	close(pipes[*teamIndex][0]);
	exit(0);
}

void *carthreads(void *index)
{
	int teamIndex = *(int *)index;
	int carIndex = *((int *)index + 1);
	int dist = 0;
	int time = 0;
	char buffer[MAXLINE];
	malfunctionMsg msg;

	while ((cars + carIndex)->nlaps < globalConfig.numLaps)
	{
		pthread_mutex_lock(&carRace->carMutex);
		while (time == carRace->time)
		{
			pthread_cond_wait(&carRace->timeCond, &carRace->carMutex);
		}
		pthread_mutex_unlock(&carRace->carMutex);
		//receber a mensagem
		if (msgrcv(mqid, &msg, sizeof(msg)-sizeof(long), (cars + carIndex)->num, IPC_NOWAIT) > 0)
		{

			(cars + carIndex)->malfunction = 1;
			(cars + carIndex)->state = 1;
			(teams + teamIndex)->numSecurityMode++;
			(teams + teamIndex)->carBoxIndex = carIndex;
			pthread_cond_signal(&(teams + teamIndex)->boxCond);
			sprintf(buffer, "CAR %d BROKE\n", (cars + carIndex)->num);
			write(pipes[teamIndex][1], buffer, sizeof(buffer));
		}

		if ((cars + carIndex)->state == 0) // corrida
		{
			dist += (cars + carIndex)->speed;
			(cars + carIndex)->fuel -= (cars + carIndex)->consumption;
		}
		else if ((cars + carIndex)->state == 1) // seguranca
		{
			dist += (0.3 * (cars + carIndex)->speed);
			(cars + carIndex)->fuel -= (0.4 * (cars + carIndex)->consumption);
		}

		if ((cars + carIndex)->fuel <= 0)
		{ //entra em estado de desistencia
			(cars + carIndex)->state = 3;
			carRace->counterCar++;
			pthread_cond_signal(&carRace->restartTime);
			carRace->controlCar--;
			sprintf(buffer, "CAR %d RAN OUT OF FUEL\n", (cars + carIndex)->num);
			write(pipes[teamIndex][1], buffer, sizeof(buffer));
			break;
		}

		if (dist > globalConfig.dist)
		{
			
			(cars + carIndex)->nlaps++;
			if ((cars + carIndex)->nlaps == globalConfig.numLaps)
			{ //terminou a corrida
				pthread_mutex_lock(&carRace->carMutex);
				if (carRace->winner == 0)
				{
					carRace->winner = 1;
					sprintf(buffer, "CAR %d WON\n", (cars + carIndex)->num);
				}
				else
				{
					sprintf(buffer, "CAR %d FINISHED\n", (cars + carIndex)->num);
				}
				pthread_mutex_unlock(&carRace->carMutex);
				carRace->numCars--;
				carRace->controlCar--;
				(cars + carIndex)->state = 4;
				pthread_cond_signal(&carRace->restartTime);
				write(pipes[teamIndex][1], buffer, sizeof(buffer));
				break;
			}
			
			if (carRace->sigintPressed == 1)
			{
				carRace->numCars--;
				carRace->controlCar--;
				pthread_cond_signal(&carRace->restartTime);
				sprintf(buffer, "CAR %d FINISHED\n", (cars + carIndex)->num);
				write(pipes[teamIndex][1], buffer, sizeof(buffer));
				break;
			}
			dist -= globalConfig.dist;
			if ((cars + carIndex)->getFuel == 1 || (cars + carIndex)->state == 1)
			{

				if (pthread_mutex_trylock(&(teams + teamIndex)->boxMutex2) == 0)
				{
						(cars + carIndex)->state = 2;
						(teams + teamIndex)->carBoxIndex = carIndex;
						sprintf(buffer, "CAR %d HAS ENTERED BOX\n", (cars + carIndex)->num);
						write(pipes[teamIndex][1], buffer, sizeof(buffer));
						pthread_cond_signal(&(teams + teamIndex)->boxCond);
						sem_wait(box);
						pthread_mutex_lock(&(teams + teamIndex)->boxMutex);
						while ((teams + teamIndex)->box == 2)
						{
							pthread_cond_wait(&(teams + teamIndex)->boxCond, &(teams + teamIndex)->boxMutex);
						}
						pthread_mutex_unlock(&(teams + teamIndex)->boxMutex);
						pthread_mutex_unlock(&(teams + teamIndex)->boxMutex2);
						(cars + carIndex)->state = 0;
						time = carRace->time - 1;
						sprintf(buffer, "CAR %d HAS LEFT THE BOX\n", (cars + carIndex)->num);
						write(pipes[teamIndex][1], buffer, sizeof(buffer));
					
					if (carRace->sigintPressed == 1)
					{
						carRace->numCars--;
						carRace->controlCar--;
						pthread_cond_signal(&carRace->restartTime);
						sprintf(buffer, "CAR %d FINISHED\n", (cars + carIndex)->num);
						write(pipes[teamIndex][1], buffer, sizeof(buffer));
						break;
					}
				
				}
			}
		}

		if ((cars + carIndex)->fuel <= globalConfig.dist / (cars + carIndex)->speed * (cars + carIndex)->consumption * 2 && (cars + carIndex)->state != 1)
		{
			(cars + carIndex)->state = 1;
			(teams + teamIndex)->numSecurityMode++;
			(teams + teamIndex)->carBoxIndex = carIndex;
			pthread_cond_signal(&(teams + teamIndex)->boxCond);
			sprintf(buffer, "CAR %d ENTERED SECURITY MODE DUE TO LOW FUEL\n", (cars + carIndex)->num);
			write(pipes[teamIndex][1], buffer, sizeof(buffer));
		}
		if ((cars + carIndex)->fuel <= globalConfig.dist / (cars + carIndex)->speed * (cars + carIndex)->consumption * 4 && (cars + carIndex)->getFuel != 1 && (cars + carIndex)->state != 2)
		{
			sprintf(buffer, "CAR %d IS TRYING TO ENTER BOX\n", (cars + carIndex)->num);
			write(pipes[teamIndex][1], buffer, sizeof(buffer));
			(cars + carIndex)->getFuel = 1;
		}
		#ifdef DEBUG
		printf("carro %d da equipa %s percorreu %d e tem %.2fL ja percorreu %d voltas \n", (cars + carIndex)->num, (teams + teamIndex)->nameTeam, dist, (cars + carIndex)->fuel, (cars + carIndex)->nlaps);
		#endif
		carRace->counterCar++;
		pthread_cond_signal(&carRace->restartTime);
		time++;
		
	}

	pthread_mutex_lock(&carRace->carMutex);
	while (carRace->controlCar > 0)
	{
		pthread_cond_wait(&carRace->endCond, &carRace->carMutex);
	}
	pthread_mutex_unlock(&carRace->carMutex);
	pthread_exit(NULL);
}

void malfunctionManager()
{
	pidMalfunction = getpid();
	pthread_mutex_lock(&carRace->carMutex);
	while (carRace->started == 0)
	{
		pthread_cond_wait(&carRace->startedCond, &carRace->carMutex);
	}
	pthread_mutex_unlock(&carRace->carMutex);
	int time = 0, n;

	malfunctionMsg msg;
	msg.n = 1;
	while (carRace->controlCar > 0)
	{

		pthread_mutex_lock(&carRace->malfunctionMutex);
		while ((time == carRace->time || carRace->time % globalConfig.T_damage != 0) && carRace->controlCar > 0 )
		{
			pthread_cond_wait(&carRace->startedCond, &carRace->malfunctionMutex);
		}
		pthread_mutex_unlock(&carRace->malfunctionMutex);
		time = carRace->time;
		if (carRace->numCars == 0)
		{
			break;
		}
		//gerar avarias
		srand(getpid());
		for (int i = 0; i < globalConfig.maxTeam * globalConfig.maxCarTeams; i++)
		{
			if ((cars + i)->num != -1 && (cars + i)->malfunction != 1 && (cars + i)->state != 4 && (cars + i)->state != 2)
			{
				n = (rand() % 100) + 1;
				if (n > (cars + i)->reliability)
				{
					msg.carNum = (cars + i)->num;
					msgsnd(mqid, &msg, sizeof(msg)-sizeof(long), 0);
					sem_wait(mutex);
					carRace->totalMalfunction++;
					sem_post(mutex);
				}
			}
		}
	}
	exit(0);
}

void sigint()
{

	if (getpid() == original)
	{

		writeLog("SINAL RECEIVED SIGINT\n");
		sem_wait(mutex);
		carRace->sigintPressed = 1;
		sem_post(mutex);
		statistics();
		writeLog("Simulador Closing\n");
		cleanup();
		exit(0);
	}
	exit(0);
}

void sigstp()
{
	writeLog("SINAL RECEIVED SIGTSTP\n");
	sem_post(stats);
	statistics();
}

void sigusr1()
{
	writeLog("SINAL RECEIVED SIGUSR1\n");
	sem_wait(mutex);
	carRace->sigusr1Pressed = 1;
	sem_post(mutex);
	wait(NULL);
	wait(NULL);
	statistics();
}

void *timeCounter()
{
	//char buffer[MAXLINE];
	pthread_mutex_lock(&carRace->carMutex);
	while (carRace->started == 0)
	{
		pthread_cond_wait(&carRace->startedCond, &carRace->carMutex);
	}
	pthread_mutex_unlock(&carRace->carMutex);
	int time = 1000000 / globalConfig.numtime; //microsegundos
	while (carRace->controlCar > 0)
	{
		carRace->time += 1;
		if (carRace->time != 0 && carRace->time % globalConfig.T_damage == 0)
		{
			pthread_cond_signal(&carRace->startedCond);
		}
		usleep(time);
		pthread_cond_broadcast(&carRace->timeCond);
		//sprintf(buffer, "tempo incrementado %d\n", carRace->time);
		//writeLog(buffer);
		pthread_mutex_lock(&carRace->carMutex);
		//espera pelos carros
		while (carRace->counterCar < carRace->numCars)
		{
			pthread_cond_wait(&carRace->restartTime, &carRace->carMutex);
		}
		pthread_mutex_unlock(&carRace->carMutex);
		carRace->counterCar = 0;
	}
	pthread_exit(NULL);
}

void statistics()
{
	char buffer[MAXLINE];
	pthread_mutex_lock(&carRace->carMutex);
	sprintf(buffer, "STATISTICS\nNUMBER OF CARS RUNNING: %d\nNUMBER OF MALFUNCTIONS: %d\nNUMBER OF REFILLS: %d\n", carRace->numCars, carRace->totalMalfunction, carRace->totalRefill);
	car auxArray[globalConfig.maxCarTeams * globalConfig.maxTeam];
	for (int i = 0; i < globalConfig.maxCarTeams * globalConfig.maxTeam; i++)
	{
		auxArray[i] = *(cars + i);
	}
	pthread_mutex_unlock(&carRace->carMutex);

	car aux;
	int teamIndex = 0;
	for (int i = 0; i < globalConfig.maxTeam * globalConfig.maxCarTeams - 1; i++)
	{
		for (int j = 0; j < globalConfig.maxTeam * globalConfig.maxCarTeams - i - 1; j++)
		{
			if (auxArray[j].nlaps < auxArray[j + 1].nlaps)
			{
				aux = auxArray[j];
				auxArray[j] = auxArray[j + 1];
				auxArray[j + 1] = aux;
			}
		}
	}
	sem_wait(stats);
	int nCar = 0;
	writeLog(buffer);
	for (int i = 0; i < globalConfig.maxCarTeams * globalConfig.maxTeam; i++)
	{
		if (nCar == 5)
			break;
		if (auxArray[i].num != -1)
		{
			teamIndex = (int)floor(i / globalConfig.maxCarTeams);
			sprintf(buffer,"CAR: %d TEAM: %s LAPS: %d BOX STOPS: %d\n", (auxArray + i)->num, (teams + teamIndex)->nameTeam, (auxArray + i)->nlaps, (auxArray + i)->numBoxStop);
			nCar++;
			writeLog(buffer);
		}
	}

	for (int i =  globalConfig.maxCarTeams * globalConfig.maxTeam -1; i > -1; i--)
	{
	
		if (auxArray[i].num != -1)
		{
			teamIndex = (int)floor(i / globalConfig.maxCarTeams);
			sprintf(buffer,"LAST: CAR: %d TEAM: %s LAPS: %d BOX STOPS: %d\n", (auxArray + i)->num, (teams + teamIndex)->nameTeam, (auxArray + i)->nlaps, (auxArray + i)->numBoxStop);
			writeLog(buffer);
			break;
		}
	}
}
