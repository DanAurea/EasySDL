

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "esave.h"
#include "aes.h"
#include "db_lite.h"
#include "cstring.h"

d_save * initProfil(char * filename) {
/* Load profil file or start new one */
	
	if (!db_open(filename)) return NULL;
	d_save * tmp = NULL;
	unsigned char ** sqlAnswer = NULL;
	char * Rparam = NULL;
	int sizesqlAnser = 0, i = 0;
	
	/* Allocate and init new save bloc */
	tmp = malloc(sizeof(d_save));
	tmp->filename = filename;
	tmp->data = NULL;
	tmp->elem = 0;
	
	while (readText(db, i, sqlAnswer, &sizesqlAnser)) {
		
		Rparam = malloc(sizeof(char)*(strlen((char*)sqlAnswer[0])+1));
		strcpy(Rparam, (char*) sqlAnswer[0]);
		
		readBlob(db, Rparam, sqlAnswer, &sizesqlAnser);
		writeParam(tmp, Rparam, (char*) sqlAnswer[0]);
		
		free(Rparam);
		
		i++;
	}
	
	db_close();
	return tmp;
}

char * readParam(d_save * profil, char * param) {

	if (!profil) return NULL;
	int len = (int) strlen(param)+1, i = 0;
	char * plaintext = NULL;
	
	for (i = 0; i < (profil->elem); i++) {
		
		if (!strcmp(plaintext, param)) {
			
			plaintext = (char *)aes_decrypt(&de, profil->data[i].value, &len);
			return plaintext;
			
		}
		
	}
	
	return NULL;
}

int writeParam(d_save * profil, char * param, char * value) {
	
	if (!profil) return 0;
	int len = (int) strlen(param)+1, i = 0;
	unsigned char *encryptedval = NULL;
	
	//Check if param is already in here.
	for (i = 0; i < (profil->elem); i++) {
		
		/* If we have a match.. */
		if (!strcmp(profil->data[i].param, param) == 0) {
			
			encryptedval = aes_encrypt(&en, (unsigned char*)value, &len);
			unsignedchar_memcpy(profil->data[i].value, encryptedval, sizeof(unsigned char)*len);
			
			return 1;
		}
		
	}
	
	//If not create new one.
	if (profil->data) {
		profil->data = realloc(profil->data, sizeof(d_param)*((profil->elem)+1));
	}else{
		profil->data = malloc(sizeof(d_param));
	}
	
	encryptedval = aes_encrypt(&en, (unsigned char*)value, &len);
	
	profil->data[profil->elem].param = malloc(sizeof(char)*strlen(param)+1);
	profil->data[profil->elem].value = malloc(sizeof(unsigned char)*sizeof(unsigned char)*len);
	
	memcpy (profil->data[profil->elem].param, param, strlen(param)+1);
	unsignedchar_memcpy(profil->data[profil->elem].value, encryptedval, sizeof(unsigned char)*len);
	
	profil->elem = (profil->elem)+1;
	
	return 1;
}

int saveProfil(d_save * profil) {
	
	if (!profil) return 0;
	if (!(profil->filename)) return 0;
	if (!db_open(profil->filename)) return 0;
	
	int i = 0;
	
	for (i = 0; i < (profil->elem); i++) {
		
		writeBlob(db, profil->data[i].param, profil->data[i].value, sizeof(unsigned char)*strlen((char*)profil->data[i].value));
		
	}
	
	db_close();
	
	return 1;
	
}
