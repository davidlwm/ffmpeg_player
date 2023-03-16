#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ability.h"

int parsingAbility(char *ability_file, AbilityInfo* ability_info)
{
    char *buf = NULL;
    int ret = 0;

    memset(ability_info, 0, sizeof(AbilityInfo));
    ret = readAbility(ability_file, &buf);
    if(ret < 0)
        return -1;

    // It's an option for sample demo, user can implement customized ability file

    releaseAbility(&buf);
    return 0;
}

int readAbility(char *ability_file, char ** ability_string)
{
    char *buf = NULL;
    FILE *fp = NULL;
    int len = 0, ret = 0;

    *ability_string = NULL;

    //Read ablility json from file
    fp = fopen(ability_file, "r");
    if(fp == NULL){
        printf("[%s:%d] ability_file:[%s] not exist\n", __FUNCTION__, __LINE__, ability_file);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    buf = (char*)malloc(len+1);
    if(buf == NULL){
        printf("[%s:%d] malloc error\n", __FUNCTION__, __LINE__);
        fclose(fp);
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    ret = fread(buf, 1, len, fp);
    if(ret != len){
        printf("[%s:%d] fread error\n", __FUNCTION__, __LINE__);
        free(buf);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    buf[len] = '\0';

    *ability_string = buf;
    return len;
}

void releaseAbility(char **ability_file)
{
    free(*ability_file);
    *ability_file = NULL;
}

