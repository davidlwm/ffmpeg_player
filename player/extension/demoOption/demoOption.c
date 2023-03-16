#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "demoOption/demoOption.h"

#define DEMO_CONFIG_FILE "config.txt"

static struct option long_options[] =
{
    {"uid",     required_argument,  0, 'u'},
    {"dtls",    no_argument,        0, 's'},
    {"token",   no_argument,        0, 't'},
    {"ability", no_argument,        0, 'a'},
    {"changepw",no_argument,        0, 'c'},
    {"newpw",   required_argument,  0, 'p'},
    {"delid",   required_argument,  0, 'd'},
    {"listid",  no_argument,        0, 'l'},
    {"search",  no_argument,        0, 'f'},
    {"newtoken",no_argument,        0, 'n'},
    {"newid",   required_argument,  0, 'i'},
    {"help",    no_argument,        0, 'h'},
    {0, 0, 0, 0}
};

static char *long_options_help_string[] = {
    " [UID]\t (M)Device UID",
    "\t\t (O)Enable connect with dtls, default is Disable",
    "\t\t (O)Enable connect with av token, default is Disable",
    "\t\t (O)Demo request ability file from device",
    "\t\t (S)Demo Change Password",
    " [password]     Input new password for demo change password",
    " [identity]\t (S)Demo delete identity",
    "\t\t (S)Demo show identity list",
    "\t\t (S)Demo lan search",
    "\t\t (S)Demo create new token with identity",
    " [identity]     Input new identity for demo create new token or demo binding device",
    "\t\t    Print help",
    ""
};

void help(char* program_name)
{
    int option = 0;

    printf("Usage: %s [options...]\nOptions: (M)Must (O)Option (S)Single Choice\n", program_name);
    while(1){
        if(long_options[option].name == NULL)
            break;
        printf(" -%c, --%s%s\n", long_options[option].val, long_options[option].name, long_options_help_string[option]);
        option++;
    }
}

int parsionOptions(int argc, char **argv, DemoOptions* options)
{
    int c;

    memset(options, 0 ,sizeof(DemoOptions));
    strncpy(options->uid, "CVV67ZVHZMZ2Z9D8111A", 20);

    if(options->demo_change_password + options->demo_delete_idenity +
       options->demo_list_identity + options->demo_lan_search_devices + options->demo_create_new_token > 1){
        printf("ERROR : You can only choice a demo\n");
        exit(0);
    }

    if (strlen(options->uid) < 20) {
        printf("ERROR : Please enter -u UID (20 characters)\n");
        exit(0);
    }

    if (options->demo_change_password && strlen(options->new_pass) == 0) {
        printf("ERROR : -c demo change password need -p NEW_PASS\n");
        exit(0);
    }

    if (options->demo_create_new_token && strlen(options->new_identity) == 0) {
        printf("ERROR : -n demo create new token with identity need -i NEW_IDENTITY\n");
        exit(0);
    }

    printf("===== demo options =====\n");
    printf("Connect to UID[%s]\n", options->uid);
    if(options->enable_dtls)printf("Enable dtls\n");
    if(options->demo_request_ability)printf("Demo request server ability\n");
    if(options->enable_connect_with_token)printf("Enable connect with av token\n");
    if(options->demo_change_password)printf("Demo Change Password to NEW_PASS[%s]\n", options->new_pass);
    if(options->demo_delete_idenity)printf("Demo delete identity [%s]\n", options->del_identity);
    if(options->demo_list_identity)printf("Demo show identity list\n");
    if(options->demo_lan_search_devices)printf("Demo lan search\n");
    if(options->demo_create_new_token)printf("Demo create new token with identity NEW_IDENTITY[%s]\n", options->new_identity);
    printf("========================\n");

    return 0;
}

int saveConfig(DemoConfig* demo_config)
{
    FILE* fp = NULL;
    fp = fopen(DEMO_CONFIG_FILE, "w");
    if (fp == NULL)
        return -1;

    fprintf(fp, "iotc_auth_key:%s\n",  demo_config->iotc_auth_key);
    fprintf(fp, "av_account:%s\n",  demo_config->av_account);
    fprintf(fp, "av_password:%s\n",  demo_config->av_password);
    fprintf(fp, "av_identity:%s\n",  demo_config->av_identity);
    fprintf(fp, "av_token:%s\n",  demo_config->av_token);

    fclose(fp);

    return 0;
}

int loadConfig(DemoConfig* demo_config)
{
    char str_line[MAX_TOKEN_LENGTH+15] = {0};
    char symbol[15] = {0};
    char content[MAX_TOKEN_LENGTH] = {0};
    FILE* fp = NULL;

    fp = fopen(DEMO_CONFIG_FILE, "r");
    if (fp != NULL) {
        while (fgets(str_line, MAX_TOKEN_LENGTH + 10, fp)!=NULL)
        {
            sscanf(str_line, "%[^:]:%s", symbol, content);
            if (!strcmp(symbol, "iotc_auth_key"))
            {
                strcpy(demo_config->iotc_auth_key, content);
            }
            else if (!strcmp(symbol, "av_account"))
            {
                strcpy(demo_config->av_account, content);
            }
            else if (!strcmp(symbol, "av_password"))
            {
                strcpy(demo_config->av_password, content);
            }
            else if (!strcmp(symbol, "av_identity"))
            {
                strcpy(demo_config->av_identity, content);
            }
            else if (!strcmp(symbol, "av_token"))
            {
                strcpy(demo_config->av_token, content);
            }
        }
        fclose(fp);

    } else {
        //default
        memset(demo_config, 0, sizeof(DemoConfig));
        strcpy(demo_config->iotc_auth_key, "00000000");
        strcpy(demo_config->av_account, "admin");
        strcpy(demo_config->av_password, "888888");
        strcpy(demo_config->av_identity, "admin");
        strcpy(demo_config->av_token, "888888");
        if (saveConfig(demo_config) < 0) {
            printf("[%s:%d] set default failed\n", __FUNCTION__, __LINE__);
            return -1;
        }
    }

    printf("===== demo configs =====\n");
    printf("iotc_auth_key [%s]\n", demo_config->iotc_auth_key);
    printf("av_account    [%s]\n", demo_config->av_account);
    printf("av_password   [%s]\n", demo_config->av_password);
    printf("av_identity   [%s]\n", demo_config->av_identity);
    printf("av_token      [%s]\n", demo_config->av_token);
    printf("========================\n");

    return 0;
}

