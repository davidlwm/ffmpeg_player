#ifndef __DEMO_OPTIONS_H__
#define __DEMO_OPTIONS_H__

#include "IOTCAPIs/IOTCCommon.h"
#include "AVAPIs/AVCommon.h"

typedef struct {
    int enable_dtls;
    int enable_connect_with_token;
    int demo_change_password;
    int demo_delete_idenity;
    int demo_list_identity;
    int demo_lan_search_devices;
    int demo_create_new_token;
    int demo_request_ability;
    char new_pass[NEW_MAXSIZE_VIEWPWD+1];       //for demo_change_password, demo_binding_device
    char del_identity[MAX_IDENTITY_LENGTH+1];   //for demo_delete_idenity
    char new_identity[MAX_IDENTITY_LENGTH+1];   //for demo_create_new_token, demo_binding_device
    char uid[24];                               //must have
} DemoOptions;

typedef struct {
    char iotc_auth_key[IOTC_AUTH_KEY_LENGTH+4];
    char av_account[NEW_MAXSIZE_VIEWACC];       //for demo_change_password
    char av_password[NEW_MAXSIZE_VIEWPWD];       //for demo_change_password
    char av_identity[MAX_IDENTITY_LENGTH+1];
    char av_token[MAX_TOKEN_LENGTH+4];
} DemoConfig;

int parsionOptions(int argc, char **argv, DemoOptions* options);
int saveConfig(DemoConfig* demo_config);
int loadConfig(DemoConfig* demo_config);

#endif

