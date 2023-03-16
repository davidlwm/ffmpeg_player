#include <sys/time.h>
#include <stddef.h>
#include <stdio.h>
#include "token.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_TOKEN_COUNT 50

typedef struct TokenData {
    char token[TOKEN_LENGTH + 1];
    char identity[MAX_IDENTITY_LENGTH + 1];
    TokenPrivilege privilege;
}TokenData;

typedef struct TokenDataPool {
    TokenData token_data[MAX_TOKEN_COUNT];
    int token_count;
}TokenDataPool;

TokenDataPool gTokenPool;
const char *gFilePath = NULL;

int SearchIdentity(const char *identity, int *index){
    for(int i=0;i<gTokenPool.token_count;i++){
        if(strncmp(identity, gTokenPool.token_data[i].identity, TOKEN_LENGTH+1) == 0) {
            *index = i;
            return TOKEN_SUCCESS;
        }
    }
    return TOKEN_IDENTITY_NOT_FOUND;
}

int SaveTokenPool(const char *file_path){

    if(file_path == NULL)
        return TOKEN_WRONG_ARG;


    char *privilege_map[2] = {0};
    privilege_map[ADMIN_PRIVILEGE] = "admin";
    privilege_map[GUEST_PRIVILEGE] = "guest";

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
       perror("fopen");
       return TOKEN_FILE_OPEN_FAILED;
    }

    for(int i=0;i<gTokenPool.token_count;i++){
        fprintf(file, "%s %s %s\n",
                gTokenPool.token_data[i].identity,
                gTokenPool.token_data[i].token,
                privilege_map[gTokenPool.token_data[i].privilege]);
    }

    fclose(file);

    return TOKEN_SUCCESS;
}

int GenerateToken(unsigned int token_length, char *token) {
    char charset[] = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    struct timeval tv;
    gettimeofday(&tv, NULL);

    if(token_length == 0 || token == NULL)
        return TOKEN_WRONG_ARG;


    while (token_length-- > 0) {
        size_t index = (rand() + (int) tv.tv_sec) % (sizeof(charset) - 1);
        *token++ = charset[index];
    }

    *token = '\0';

    return TOKEN_SUCCESS;
}


int LoadTokenFile(const char *file_path){
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    char *delim = " ";
    char * pch;

    memset(&gTokenPool, 0, sizeof(gTokenPool));

    file = fopen(file_path, "r");
    if (file == NULL) {
       perror("fopen");
       return TOKEN_FILE_OPEN_FAILED;
    }

    while ((nread = getline(&line, &len, file)) != -1) {

        if(strlen(line)<3)
            continue;

        //printf("%s %d\n", line, gTokenPool.token_count);

        //get identity
        pch = strtok(line,delim);
        strcpy(gTokenPool.token_data[gTokenPool.token_count].identity, pch);
        //printf("%s %s\n", pch, gTokenPool.token_data[gTokenPool.token_count].identity);

        //get token
        pch = strtok (NULL, delim);
        strcpy(gTokenPool.token_data[gTokenPool.token_count].token, pch);
        //printf("%s\n", pch);

        //get privilege
        pch = strtok (NULL, delim);
        //printf("%s\n", pch);
        if(strcmp(pch, "admin\n")==0){
            //printf("find admin\n");
            gTokenPool.token_data[gTokenPool.token_count].privilege = ADMIN_PRIVILEGE;
        } else if(strcmp(pch, "guest\n")==0){
            //printf("find guest\n");
            gTokenPool.token_data[gTokenPool.token_count].privilege = GUEST_PRIVILEGE;
        } else {
            //printf("find nothing\n");
            return TOKEN_WRONG_FORMAT;
        }
        gTokenPool.token_count ++;
    }

    free(line);
    fclose(file);

#if 0
    for(int i=0;i<gTokenPool.token_count;i++){
        printf("%s\n %s\n %d\n", gTokenPool.token_data[i].identity,
               gTokenPool.token_data[i].token,
               gTokenPool.token_data[i].privilege);
    }
#endif
    if(gTokenPool.token_count == 0)
        return TOKEN_NO_TOKEN;


    gFilePath = file_path;

    return TOKEN_SUCCESS;
}


int GetPrivilege(const char *identity, TokenPrivilege *privilege){
    if(identity == NULL || privilege == NULL)
        return TOKEN_WRONG_ARG;

    for(int i=0;i<gTokenPool.token_count;i++){
        if(strcmp(gTokenPool.token_data[i].identity, identity)==0){
            *privilege = gTokenPool.token_data[i].privilege;
            return TOKEN_SUCCESS;
        }
    }
    return TOKEN_IDENTITY_NOT_FOUND;
}

int GetToken(const char *identity, char *token, unsigned int *token_length) {
    if(identity == NULL || token == NULL || token_length == NULL)
        return TOKEN_WRONG_ARG;

    for(int i=0;i<gTokenPool.token_count;i++){
        if(strcmp(gTokenPool.token_data[i].identity, identity)==0){
            strncpy(token, gTokenPool.token_data[i].token, TOKEN_LENGTH+1);
            *token_length = strlen(gTokenPool.token_data[i].token);
            return TOKEN_SUCCESS;
        }
    }
    return TOKEN_IDENTITY_NOT_FOUND;
}

int CreateTokenWithIdentity(
        const char *identity,
        unsigned int identity_length,
        unsigned int token_length,
        char *token,
        TokenPrivilege privilege){

    if(identity == NULL || identity_length == 0 ||
       token == NULL)
        return TOKEN_WRONG_ARG;

    int index;
    if(SearchIdentity(identity, &index) == TOKEN_SUCCESS)
        return TOKEN_IDENTITY_DUPLICATED;

    TokenData *data = &gTokenPool.token_data[gTokenPool.token_count];
    strncpy(data->identity, identity, identity_length+1);

    int ret = GenerateToken(token_length, data->token);
    if(ret < 0)
        return ret;

    strncpy(token, data->token, token_length+1);

    data->privilege = privilege;

    gTokenPool.token_count++;

    ret = SaveTokenPool(gFilePath);
    if(ret < 0)
        return ret;

    return TOKEN_SUCCESS;
}

int DeleteTokenByIdentity(
        const char *identity,
        unsigned int identity_length){

    if(identity == NULL || identity_length == 0)
        return TOKEN_WRONG_ARG;

    for(int i=0;i<gTokenPool.token_count;i++){
        if(strcmp(gTokenPool.token_data[i].identity, identity)==0){
            if(i != gTokenPool.token_count-1){
                strcpy(gTokenPool.token_data[i].identity,gTokenPool.token_data[gTokenPool.token_count-1].identity);
                strcpy(gTokenPool.token_data[i].token,gTokenPool.token_data[gTokenPool.token_count-1].token);
                gTokenPool.token_data[i].privilege = gTokenPool.token_data[gTokenPool.token_count-1].privilege;
            }
            memset(gTokenPool.token_data[gTokenPool.token_count-1].identity,0,MAX_IDENTITY_LENGTH+1);
            memset(gTokenPool.token_data[gTokenPool.token_count-1].token,0,TOKEN_LENGTH+1);

            gTokenPool.token_count--;

            int ret = SaveTokenPool(gFilePath);
            if(ret < 0)
                return ret;

            return TOKEN_SUCCESS;
        }
    }

    return TOKEN_IDENTITY_NOT_FOUND;
}

int GetIdentityArray(
        AvIdentity **identities,
        unsigned int *identity_count){

    if(identities==NULL || identity_count==NULL)
        return TOKEN_WRONG_ARG;

    if(gTokenPool.token_count == 0)
        return TOKEN_IDENTITY_IS_EMPTY;

    AvIdentity *identities_tmp = (AvIdentity *)malloc(sizeof(AvIdentity)*gTokenPool.token_count);
    for(int i=0;i<gTokenPool.token_count;i++){
        strncpy(identities_tmp[i].identity, gTokenPool.token_data[i].identity,MAX_IDENTITY_LENGTH+1);
    }

    *identity_count = gTokenPool.token_count;
    *identities = identities_tmp;

    return TOKEN_SUCCESS;
}

int CleanIdentities(){
    memset(&gTokenPool, 0, sizeof(gTokenPool));

    int ret = SaveTokenPool(gFilePath);
    if(ret<0)
        return ret;

    return TOKEN_SUCCESS;
}

int FreeIdentityArray(AvIdentity *identities){
    free(identities);
    return TOKEN_SUCCESS;
}
