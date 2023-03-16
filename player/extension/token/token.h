#ifndef __AV_TOKEN_SAMPLE__
#define __AV_TOKEN_SAMPLE__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "AVAPIs/AVCommon.h"

#define TOKEN_LENGTH 64

#ifndef MAX_IDENTITY_LENGTH
#define MAX_IDENTITY_LENGTH 119
#endif

#define TOKEN_SUCCESS 0

#define TOKEN_WRONG_ARG -1

#define TOKEN_WRONG_FORMAT -2

#define TOKEN_FILE_OPEN_FAILED -3

#define TOKEN_NO_TOKEN -4

#define TOKEN_IDENTITY_NOT_FOUND -5

#define TOKEN_IDENTITY_DUPLICATED -6

#define TOKEN_IDENTITY_IS_EMPTY -7

typedef enum TokenPrivilege {
    GUEST_PRIVILEGE,
    ADMIN_PRIVILEGE
}TokenPrivilege;

int GenerateToken(
        unsigned int token_length,
        char *token);

int CreateTokenWithIdentity(
        const char *identity,
        unsigned int identity_length,
        unsigned int token_length,
        char *token,
        TokenPrivilege privilege);

int DeleteTokenByIdentity(
        const char *identity,
        unsigned int identity_length);

int CleanIdentities();

int GetPrivilege(
        const char *identity,
        TokenPrivilege *privilege);

int GetToken(
        const char *identity,
        char *token,
        unsigned int *token_length);

int GetIdentityArray(
        AvIdentity **identities,
        unsigned int *identity_count);

int FreeIdentityArray(
        AvIdentity *identities);

int LoadTokenFile(const char *file_path); //create token file automatically

int SaveTokenPool(const char *file_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __AV_TOKEN_SAMPLE__
