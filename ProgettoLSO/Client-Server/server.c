#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<signal.h>

#include "../Database/sql.c"

#define PORT_N 9876

void* threadFunction(void* socketDescriptor);
void getRequest(int socketDescriptor);
void postRequest(int socketDescriptor);

void signUSRHandlet(int signal);

static int socketDescriptor;

int main(int argc, char* argv[]){

	//Dichiaro l'handler del segnalre USR1, in modo da avere un modo 'safe' di chiudere il server
	signal(SIGUSR2,signUSRHandlet); 

	int clientSocket, *new_sock;

	//Struttura dati sockets
	struct sockaddr_in server , client;

	char* hostname = "0.0.0.0";

	if( ( socketDescriptor = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ){

		perror("Errore nella creazione della socket\n");
		exit(1);

	}

	//Assegno i valori alla struttura 'server'
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(hostname);
	server.sin_port = htons(PORT_N);


	//Avverto il sistema operativo che voglio che la socket sia 'riconosciuta' e entri in funzione
	if( ( bind(socketDescriptor, (struct sockaddr*) &server, sizeof(server)) ) < 0 ){

		perror("Errore nel binding\n");
		exit(1);

	}

	printf("[*] Binding effettuato con successo\n");

	//Inizio effettivamente ad ascoltare. Da questo momento in poi il server è aperto

	listen(socketDescriptor, 10);

	printf("[*] Sono in ascolto su %s : %d. Attendo nuove connessioni...\n", hostname, PORT_N);

	int c = sizeof(struct sockaddr_in);


	while( (clientSocket = accept(socketDescriptor, (struct sockaddr*)&clientSocket, (socklen_t*)&c) ) > 0){


		printf("[*] Connessione stabilita, gestisco la richiesta\n");

		//Alloco il thread
		pthread_t thread;
		//Alloco memoria per la copia del socket che darò al thread
		new_sock = malloc(sizeof(int));
		//Copio
		*new_sock = clientSocket;

		if( pthread_create(&thread, NULL, threadFunction, (void*) new_sock) < 0 ){

			perror("Errore nella creazione di un thread\n");

		}

	}

	printf("[*] Server out!\n");
	close(socketDescriptor);
	return 0;
}

void* threadFunction(void* socketDescriptor){

	int socketDesc = *(int*)socketDescriptor;
	char requestBuffer[50];

	printf("[*] Connessione effettuata con il client\n");
	//Scrivo per primo al client

	printf("[*] Lettura dati dal client\n");
	recv(socketDesc, requestBuffer, 50, 0);

	if(strcmp(requestBuffer, "get") == 0){

		printf("[*] Richiesta get - Invio i dati al client\n");
		getRequest(socketDesc);

	}else if(strcmp(requestBuffer, "post") == 0){

		printf("[*] Richiesta post - Mi preparo a ricevere dati\n");
		postRequest(socketDesc);

	}else if(strcmp(requestBuffer, "toll") == 0){

		send(socketDesc, "0.000002;", 8, 0);

	}else{

		close(socketDesc);

	}

	close(socketDesc);

	return 0;

}

void getRequest(int socketDescriptor){

	char readBuffer[1000];
	int byteLetti = 0, byteScritti = 0;

	//Riferimento al database in cui andrò ad inserire i dati
	sqlite3* database;

	//Leggo fin quando ha dati da mandarmi
	recv(socketDescriptor, readBuffer, 2000, 0);
	//printf("%s\n", readBuffer);


	char* field = strtok(readBuffer, ";");
	char nickname[20];
	char latitude[20];
	char longitude[20];
	int charN = 0;

	while(field != NULL){

		if(charN == 0){
			strcpy(nickname, field);
		}else if (charN == 1){
			strcpy(latitude, field);
		}else if (charN == 2){
			strcpy(longitude, field);
		}
		charN++;

		field = strtok(NULL, ";");

	}

	printf("Dati su cui sto andando a fare la ricerca: %s - %f - %f\n", nickname, atof(latitude), atof(longitude));

	//Chiedo alla funzione di aprire il database chiamato 'potholes'
	int status = sqlite3_open_v2("potholes.db", &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, NULL);

	if(status == SQLITE_OK){

		status = getPotholes(database, atof(latitude), atof(longitude), socketDescriptor);

		if(status == SQLITE_OK){

			printf("[*] Ricerca avvenuta con successo\n");

		}else{

			printf("[*] Ricerca fallita - %d\n", status);

		}

	}else{

		printf("Apertura del database fallita\n");

	}

	//Mi congedo da client
	printf("[*] Comunicazione terminata, chiudo la connessione\n");

	sqlite3_close(database);

	return;

}

void postRequest(int socketDescriptor){

	char readBuffer[1000];
	int byteLetti = 0, byteScritti = 0;

	//Riferimento al database in cui andrò ad inserire i dati
	sqlite3* database;

	//Leggo fin quando ha dati da mandarmi
	recv(socketDescriptor, readBuffer, 2000, 0);
	//printf("%s\n", readBuffer);


	char* field = strtok(readBuffer, ";");
	char nickname[20];
	char latitude[20];
	char longitude[20];
	int charN = 0;

	while(field != NULL){

		if(charN == 0){
			strcpy(nickname, field);
		}else if (charN == 1){
			strcpy(latitude, field);
		}else if (charN == 2){
			strcpy(longitude, field);
		}
		charN++;

		field = strtok(NULL, ";");

	}

	//Chiedo alla funzione di aprire il database chiamato 'potholes'
	int status = sqlite3_open_v2("potholes.db", &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, NULL);

	if(status == SQLITE_OK){

		status = insertPothole(database, nickname, atof(latitude), atof(longitude));

		if(status == SQLITE_OK){

			printf("[*] Inserimento avvenuto con successo\n");

		}else{

			printf("[!] Inserimento fallito - %d\n", status);

		}

	}else{

		printf("[!] Apertura del database fallita\n");

	}

	//Mi congedo da client
	printf("[*] Comunicazione terminata, chiudo la connessione\n");

	sqlite3_close(database);

	return;

}


void signUSRHandlet(int signal){

	printf("[*] Ricevuto segnale USR, procedo alla chiusura del server.\n");
	close(socketDescriptor);
	printf("[*] Server out!\n");
	exit(0);

}



