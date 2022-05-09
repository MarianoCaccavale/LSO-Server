#include <stdio.h>
#include <stdlib.h>

#include "../Database/sql.c"

int main(int argc, char* argv[]){

	int status = 0;
	sqlite3* database;
	
	status = sqlite3_open_v2("potholes.db", &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| SQLITE_OPEN_NOMUTEX, NULL);
	
	if(status != SQLITE_OK){
	
		printf("[!] Errore nell'apertura/creazione del database, stato %d\n", status);
		exit(1);
	}else{
	
		printf("[*] Apertura/creazione del database effettuata con successo\n");
	
	}
	
	char* query = "CREATE TABLE IF NOT EXISTS potholes (nickname VARCHAR(25), latitude DOUBLE, longitude DOUBLE);";

	status = sqlite3_exec(database, query, NULL, NULL, NULL);
	
	if(status != SQLITE_OK){
	
		printf("[!] Errore nella creazione della tabella, stato %d\n", status);
		exit(1);
	}else{
	
		printf("[*] Creazione della tabella avvenuta con successo\n");
	
	}

	query = "insert into potholes values ('Alice', '40.837520', '14.185963')";

	status = sqlite3_exec(database, query, NULL, NULL, NULL);

	if(status != SQLITE_OK){
	
		printf("[!] Errore nella creazione del primo record, stato %d\n", status);

	}else{
	
		printf("[*] Inserimento avvenuto con successo\n");
	
	}
	
	query = "insert into potholes values ('Bob', '40.839160', '14.188377')";

	status = sqlite3_exec(database, query, NULL, NULL, NULL);

	if(status != SQLITE_OK){
	
		printf("[!] Errore nella creazione del secondo record, stato %d\n", status);

	}else{
	
		printf("[*] Inserimento avvenuto con successo\n");
	
	}
	
	printf("[*] Set-up completato con successo\n");
	return 0;


}
