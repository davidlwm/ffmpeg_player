#ifndef _ABILITY_
#define _ABILITY_

typedef struct
{
    unsigned char dtls;
    unsigned char iotc_auth;
    unsigned char av_token;
    unsigned char full_duplex;
    unsigned char dasa;
} AbilityInfo;

int parsingAbility(char *ability_file, AbilityInfo* ability_info);
int readAbility(char *ability_file, char ** ability_string);
void releaseAbility(char **ability_string);

#endif

