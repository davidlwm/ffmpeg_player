// tests.cpp
#include "token.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <string.h>

const char * token_file_content =
"terry oEIYSTkq8AYFCMmkabpEwBHXXmFsWFrtf4l1ROAVwsu2yYiE4B1i9Qz0k8bpKwMT admin\n\
\n\
mary NwCQ8xoMd6AW8MCuSgkF2Kzar2T9jPcOgxMic5Mj5gb7aHjWGYkQQ1UcXJ3ZtoVr guest\n\
mimi JE7bnFGg9V2hWF3E22WG3o51k6UbNRLqEMvVz85QX31PqZoAJsbULo4pcSITRpsD guest\n\
";

void OverWriteTokenFile(const char *path){
    FILE * file = fopen(path, "w");
    fprintf( file, "%s\n",token_file_content);
    fclose (file);
}

class TestToken : public ::testing::Test {
    virtual void SetUp(){
        token_file = "token.txt";
        OverWriteTokenFile(token_file);
        LoadTokenFile(token_file);
    }

    virtual void TearDown(){

    }

public:
    const char *token_file;
    void TestGetTokenImpl(){
        char token[TOKEN_LENGTH];
        unsigned int token_length;
        int ret = GetToken("terry", token, &token_length);
        ASSERT_EQ(ret, TOKEN_SUCCESS);
        ASSERT_STREQ(token, "oEIYSTkq8AYFCMmkabpEwBHXXmFsWFrtf4l1ROAVwsu2yYiE4B1i9Qz0k8bpKwMT");

        ret = GetToken("mary", token, &token_length);
        ASSERT_EQ(ret, TOKEN_SUCCESS);
        ASSERT_STREQ(token, "NwCQ8xoMd6AW8MCuSgkF2Kzar2T9jPcOgxMic5Mj5gb7aHjWGYkQQ1UcXJ3ZtoVr");

        ret = GetToken("mimi", token, &token_length);
        ASSERT_EQ(ret, TOKEN_SUCCESS);
        ASSERT_STREQ(token, "JE7bnFGg9V2hWF3E22WG3o51k6UbNRLqEMvVz85QX31PqZoAJsbULo4pcSITRpsD");
     }
};

TEST_F(TestToken, TestGetIdentityArrayWrongARG) {
    AvIdentity *identities = NULL;
    unsigned int identity_count = 0;
    int ret = GetIdentityArray(NULL, &identity_count);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);

    ret = GetIdentityArray(&identities, NULL);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);
}

TEST_F(TestToken, TestGetIdentityArrayWithEmptyIdentity) {
    AvIdentity *identities = NULL;
    unsigned int identity_count = 0;
    int ret = DeleteTokenByIdentity("terry", strlen("terry"));
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ret = DeleteTokenByIdentity("mary", strlen("mary"));
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ret = DeleteTokenByIdentity("mimi", strlen("mimi"));
    ASSERT_EQ(ret, TOKEN_SUCCESS);

    ret = GetIdentityArray(&identities, &identity_count);
    ASSERT_EQ(ret, TOKEN_IDENTITY_IS_EMPTY);
}

TEST_F(TestToken, TestGetIdentityArray) {
    AvIdentity *identities = NULL;
    unsigned int identity_count = 0;
    int ret = GetIdentityArray(&identities, &identity_count);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_EQ(identity_count, 3);

    ASSERT_STREQ(identities[0].identity, "terry");
    ASSERT_STREQ(identities[1].identity, "mary");
    ASSERT_STREQ(identities[2].identity, "mimi");

    ret = FreeIdentityArray(identities);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
}

TEST_F(TestToken, TestCleanIdentities) {
    AvIdentity *identities = NULL;
    unsigned int identity_count = 0;

    int ret = CleanIdentities();
    ASSERT_EQ(ret, TOKEN_SUCCESS);

    ret = GetIdentityArray(&identities, &identity_count);
    ASSERT_EQ(ret, TOKEN_IDENTITY_IS_EMPTY);
}

TEST_F(TestToken, TestDeleteTokenWithWrongARG) {
    int ret = DeleteTokenByIdentity(NULL, 0);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);
}

TEST_F(TestToken, TestDeleteTokenWithWrongIdentity) {
    const char *identity = "bill";
    int ret = DeleteTokenByIdentity(identity, strlen(identity));
    ASSERT_EQ(ret, TOKEN_IDENTITY_NOT_FOUND);
}

TEST_F(TestToken, TestDeleteTokenByIdentity) {
    const char *identity = "terry";
    char token[TOKEN_LENGTH+1] = {0};
    unsigned int token_length;
    int ret = DeleteTokenByIdentity(identity, strlen(identity));
    ASSERT_EQ(ret, TOKEN_SUCCESS);

    ret = GetToken("terry", token, &token_length);
    ASSERT_EQ(ret, TOKEN_IDENTITY_NOT_FOUND);

    ret = GetToken("mary", token, &token_length);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_STREQ(token, "NwCQ8xoMd6AW8MCuSgkF2Kzar2T9jPcOgxMic5Mj5gb7aHjWGYkQQ1UcXJ3ZtoVr");

    LoadTokenFile(token_file);

    ret = GetToken("terry", token, &token_length);
    ASSERT_EQ(ret, TOKEN_IDENTITY_NOT_FOUND);

    ret = GetToken("mary", token, &token_length);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_STREQ(token, "NwCQ8xoMd6AW8MCuSgkF2Kzar2T9jPcOgxMic5Mj5gb7aHjWGYkQQ1UcXJ3ZtoVr");
}

TEST_F(TestToken, TestCreateTokenWithDuplicatedIdentity) {
    TokenPrivilege privilege;
    const char *identity = "terry";
    char token[TOKEN_LENGTH+1] = {0};
    int ret = CreateTokenWithIdentity(
                    identity,
                    strlen(identity),
                    TOKEN_LENGTH,
                    token,
                    ADMIN_PRIVILEGE);
    ASSERT_EQ(ret, TOKEN_IDENTITY_DUPLICATED);
}

TEST_F(TestToken, TestCreateTokenWithIdentityWrongARG) {
    TokenPrivilege privilege;
    const char *identity = "momo";
    char token[TOKEN_LENGTH+1] = {0};
    int ret = CreateTokenWithIdentity(
                    NULL,
                    strlen(identity),
                    TOKEN_LENGTH,
                    token,
                    ADMIN_PRIVILEGE);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);

    ret = CreateTokenWithIdentity(
                    identity,
                    0,
                    TOKEN_LENGTH,
                    token,
                    ADMIN_PRIVILEGE);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);

    ret = CreateTokenWithIdentity(
                    identity,
                    strlen(identity),
                    TOKEN_LENGTH,
                    NULL,
                    ADMIN_PRIVILEGE);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);
}

TEST_F(TestToken, TestCreateTokenWithIdentity) {
    TokenPrivilege privilege;
    const char *identity = "momo";
    char token[TOKEN_LENGTH+1] = {0};
    int ret = CreateTokenWithIdentity(
                    identity,
                    strlen(identity),
                    TOKEN_LENGTH,
                    token,
                    ADMIN_PRIVILEGE);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_EQ(strlen(token), TOKEN_LENGTH);

    // check fetch directly
    char token_fetched[TOKEN_LENGTH+1] = {0};
    unsigned int token_fetched_length;
    ret = GetToken("momo", token_fetched, &token_fetched_length);
    ASSERT_STREQ(token_fetched, token);

    //check fetch from file
    LoadTokenFile(token_file);
    ret = GetToken("momo", token_fetched, &token_fetched_length);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_STREQ(token_fetched, token);
    ret = GetPrivilege("momo", &privilege);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_EQ(privilege, ADMIN_PRIVILEGE);

    TestGetTokenImpl();

    ret = GetToken("momo", token_fetched, &token_fetched_length);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_STREQ(token_fetched, token);

    ret = GetPrivilege("momo", &privilege);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_EQ(privilege, ADMIN_PRIVILEGE);
}


TEST_F(TestToken, TestSaveTokenPool) {
    int ret = SaveTokenPool("test.txt");
    ASSERT_EQ(ret, TOKEN_SUCCESS);

    ret = LoadTokenFile("test.txt");
    ASSERT_EQ(ret, TOKEN_SUCCESS);

    TestGetTokenImpl();
}

TEST_F(TestToken, TestSaveTokenPoolWrongARG) {
    int ret = SaveTokenPool(NULL);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);
}


TEST(GenerateToken, TestTokenLength) {
    char token[120];
    for(int i=0;i<120;i++)
        token[i] = 1;

    for(int i=0;i<10;i++) {
        int ret = GenerateToken(TOKEN_LENGTH, token);
        //printf("token %s\n", token);

        ASSERT_EQ(ret, TOKEN_SUCCESS);
        ASSERT_EQ(strlen(token), TOKEN_LENGTH);
    }

    for(int i=0;i<120;i++)
        token[i] = 1;

    for(int i=1;i<101;i+=10) {
        int ret = GenerateToken(i, token);
       // printf("token %s\n", token);

        ASSERT_EQ(ret, TOKEN_SUCCESS);
        ASSERT_EQ(strlen(token), i);
    }
}

TEST(GenerateToken, TestTokenLength0) {
    char token[120];
    int ret = GenerateToken(0, token);

    ASSERT_EQ(ret, TOKEN_WRONG_ARG);

    ret = GenerateToken(10, NULL);

    ASSERT_EQ(ret, TOKEN_WRONG_ARG);

}

TEST(LoadTokenFile, TestWrongPath) {
    int ret = LoadTokenFile("token1.txt");

    ASSERT_EQ(ret, TOKEN_FILE_OPEN_FAILED);
}

TEST_F(TestToken, TestTokenPrivilege) {
    TokenPrivilege privilege;
    int ret = -1;
    ret = GetPrivilege("terry", &privilege);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_EQ(privilege, ADMIN_PRIVILEGE);

    ret = GetPrivilege("mary", &privilege);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_EQ(privilege, GUEST_PRIVILEGE);

    ret = GetPrivilege("mimi", &privilege);
    ASSERT_EQ(ret, TOKEN_SUCCESS);
    ASSERT_EQ(privilege, GUEST_PRIVILEGE);
}

TEST_F(TestToken, TestGetPrivilegeIdentityNotFound) {
    TokenPrivilege privilege;
    int ret = -1;
    ret = GetPrivilege("bill", &privilege);
    ASSERT_EQ(ret, TOKEN_IDENTITY_NOT_FOUND);
}

TEST_F(TestToken, TestGetPrivilegeWIthWrongARG) {
    TokenPrivilege privilege;
    int ret = -1;
    ret = GetPrivilege(NULL, &privilege);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);

    ret = GetPrivilege("terry", NULL);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);
}

TEST_F(TestToken, TestGetToken) {
    TestGetTokenImpl();
}

TEST_F(TestToken, TestGetTokenWrongARG) {
    char token[TOKEN_LENGTH+1];
    unsigned int token_length;
    int ret = GetToken(NULL, token, &token_length);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);

    ret = GetToken("terry", NULL, &token_length);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);

    ret = GetToken("terry", token, NULL);
    ASSERT_EQ(ret, TOKEN_WRONG_ARG);
}

TEST_F(TestToken, TestGetTokenIdentityNotFound) {
    char token[TOKEN_LENGTH+1];
    unsigned int token_length;
    int ret = GetToken("bill", token, &token_length);
    ASSERT_EQ(ret, TOKEN_IDENTITY_NOT_FOUND);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
