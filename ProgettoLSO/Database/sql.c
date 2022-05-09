#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/socket.h>
#include <string.h>
#include <sqlite3.h>

#define MAX_POTHOLES 100

int insertPothole(sqlite3* database, char* nickname, double latitude, double longitude);

int getPotholes(sqlite3* database, double latitude, double longitude, int socket);

/*Funzione di inserimento delle buche. Prende in input il database in cui inserire(database), il
nickname dell'utente che ha rilevato la buca(nickname), la latitudine e longitudine della buca(latitude & longitude)
Valore di ritorno: INTERO, risultato della computazione dell'insert rispetto alle costanti SQLITE 
*/
int insertPothole(sqlite3* database, char* nickname, double latitude, double longitude){

	int status;

	char* query = sqlite3_mprintf("insert into potholes values ('%q', %f, %f);", nickname, latitude, longitude);
	
	status = sqlite3_exec(database, query, NULL, NULL, NULL);

	return status;
	
}

/*Funzione di retrieve delle buche. Prende in input il database da cui prendere(database), la latitudine e longitudine nei pressi del quale cercare le buche(latitude & longitude)
Valore di ritorno: INTERO, risultato della computazione dell'insert rispetto alle costanti SQLITE 
*/
int getPotholes(sqlite3* database, double latitude, double longitude, int socket){

	int status = 0, numElem = 0;
	int byteScritti = 0;
	double minLatitude, maxLatitude, minLongitude, maxLongitude;
	char* nickname;
	char tmpValue[50];
	
	minLatitude 	= latitude-0.000005;
	maxLatitude 	= latitude+0.000005;
	minLongitude 	= longitude-0.000005;
	maxLongitude 	= longitude+0.000005;
	
	char* query_retrieve = sqlite3_mprintf("select * from potholes where latitude > '%f' AND latitude < '%f' AND longitude > '%f' AND longitude < '%f';", minLatitude, maxLatitude, minLongitude, maxLongitude);
	
	sqlite3_stmt* stmt;

	status = sqlite3_prepare_v2(database, query_retrieve, strlen(query_retrieve), &stmt, NULL);

	while((sqlite3_step(stmt) == SQLITE_ROW) && numElem <= MAX_POTHOLES){
	
	
		for ( int colIndex = 0; colIndex < sqlite3_column_count( stmt ); colIndex++ ) {
		
			if(colIndex == 0){
		        	//prima colonna, nickname
				nickname = sqlite3_column_text( stmt, colIndex );
				printf("Nickname: %s\n", nickname);
		        }else if(colIndex == 1){		        		
		        	//lat
	     			double latitude = sqlite3_column_double( stmt, colIndex );
				printf("Latitudine: %f\n", latitude);
	        	}else if(colIndex == 2){
	        		//long
	     			double longitude = sqlite3_column_double( stmt, colIndex );
				printf("Longitude: %f\n", longitude);
	        	}
		}
		numElem++;

		bzero((char*) &tmpValue, sizeof(tmpValue));

		//Invio i dati sul socket
		sprintf(tmpValue, "%s;", nickname);
		byteScritti = send(socket, tmpValue, 50, 0);
		sprintf(tmpValue, "%lf;", latitude);
		byteScritti = send(socket, tmpValue, 20, 0);
		sprintf(tmpValue, "%lf\n", longitude);
		byteScritti = send(socket, tmpValue, 20, 0);

        	printf("___________________________________________\n");

	}


	sqlite3_finalize(stmt);

}
