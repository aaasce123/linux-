#ifndef JWT_TOKEN_H
#define JWT_TOKEN_H
#include<time.h>
typedef struct {
    char* user_id;      // sub
    char* issuer;       // iss
    char* audience;     // aud
    char* username;     // name
    int is_admin;       // admin（1是管理员，0不是）
    time_t iat;         // 签发时间
    time_t exp;         // 过期时间
} Jwt_payload;
// 生成JWT Token
// 参数：
//   key: 用于签名的密钥
//   user_id: 用户唯一标识
//   username: 用户名
//   is_admin: 是否管理员（1 是，0 否）
//   expire_seconds: Token有效时间（单位：秒）
// 返回：
//   成功返回token字符串，失败返回NULL（需要调用者负责释放返回字符串）
char *jwt_token(const char *user_id,const char *username, int is_admin, int expire_seconds);

Jwt_payload* jwt_decode(const char* token);

void free_payload(Jwt_payload* payload);
void printpayload(Jwt_payload* payload);
char* my_strdup(const char* s);

#endif // JWT_TOKEN_H

