#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "jwt_token.h"
#include "l8w8jwt/algs.h"
#include "l8w8jwt/encode.h"
#include "l8w8jwt/decode.h"
#include "l8w8jwt/retcodes.h"
#include "l8w8jwt/version.h"

// 32字节的HS256密钥
static const unsigned char key[32] = {
    0x42, 0x5f, 0x9a, 0xe7, 0x7c, 0x21, 0x31, 0x95,
    0x60, 0xde, 0xa4, 0xbc, 0x02, 0x17, 0x7f, 0xaa,
    0x4d, 0x1b, 0xd4, 0x6e, 0xb2, 0xc9, 0x9d, 0xfa,
    0x6a, 0x03, 0xb3, 0xd1, 0xf1, 0x02, 0x0c, 0x77
};

/**
 * 生成 JWT Token
 * @param user_id       用户ID，非空字符串
 * @param username      用户名，非空字符串
 * @param is_admin      管理员标志，0或1
 * @param expire_seconds 过期时间，单位秒，必须大于0
 * @return              返回分配的Token字符串，调用者负责free；失败返回NULL
 */
char* jwt_token(const char* user_id, const char* username, int is_admin, int expire_seconds) {
    if (!user_id || !username || expire_seconds <= 0) {
        fprintf(stderr, "Invalid input parameters\n");
        return NULL;
    }

    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    // 设置算法和密钥
    params.alg = L8W8JWT_ALG_HS256;
    params.secret_key = (unsigned char*)key;
    params.secret_key_length = sizeof(key);

    // 获取当前时间
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        fprintf(stderr, "Failed to get current time\n");
        return NULL;
    }
    params.iat = now;
    params.exp = now + expire_seconds;

    // 设置标准声明
    params.sub =(char*) user_id;
    params.sub_length = strlen(user_id);

    params.iss = "login-server";
    params.iss_length = strlen(params.iss);

    params.aud = "api-login";
    params.aud_length = strlen(params.aud);

    // 自定义声明
    struct l8w8jwt_claim claims[2];

    // 用户名声明
    claims[0].key = "name";
    claims[0].key_length = strlen(claims[0].key);
    claims[0].value = (char*)username;
    claims[0].value_length = strlen(username);
    claims[0].type = L8W8JWT_CLAIM_TYPE_STRING;

    // 管理员状态声明
    claims[1].key = "admin";
    claims[1].key_length = strlen(claims[1].key);
    claims[1].value = is_admin ? "true" : "false";
    claims[1].value_length = strlen(claims[1].value);
    claims[1].type = L8W8JWT_CLAIM_TYPE_STRING;

    params.additional_payload_claims = claims;
    params.additional_payload_claims_count = 2;

    // 输出缓冲区
    char* jwt = NULL;
    size_t jwt_length = 0;
    params.out = &jwt;
    params.out_length = &jwt_length;

    // 编码JWT
    int r = l8w8jwt_encode(&params);
    if (r != L8W8JWT_SUCCESS) {
        fprintf(stderr, "JWT encoding failed: %d\n", r);
        return NULL;
    }

    // 复制结果，确保调用者安全释放
    char* result = malloc(jwt_length + 1);
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        l8w8jwt_free(jwt);
        return NULL;
    }
    memcpy(result, jwt, jwt_length);
    result[jwt_length] = '\0';

    // 释放库分配的内存
    l8w8jwt_free(jwt);

    return result;
}

/**
 * strdup的自定义实现
 */
char* my_strdup(const char* s) {
    if (!s) {
        return NULL;
    }
    size_t len = strlen(s);
    char* p = (char*)malloc(len + 1);
    if (p) {
        memcpy(p, s, len + 1);
    }
    return p;
}

Jwt_payload* jwt_decode(const char* token) {
    if (token == NULL) {
        return NULL;
    }

    struct l8w8jwt_decoding_params params;
    memset(&params, 0, sizeof(params));

    params.alg = L8W8JWT_ALG_HS256;
    params.jwt = (char*)token;
    params.jwt_length = strlen(token);
    params.verification_key = (unsigned char*)key;
    params.verification_key_length =sizeof(key);
    params.validate_exp = 1;
    params.exp_tolerance_seconds = 5;
    params.validate_nbf = 0;

    enum l8w8jwt_validation_result validation_result;
    struct l8w8jwt_claim* claims = NULL;
    size_t claims_count = 0;

    int ret = l8w8jwt_decode(&params, &validation_result, &claims, &claims_count);
    if (ret != 0) {
        return NULL;
    }

    if (validation_result != L8W8JWT_VALID) {
    if (validation_result == L8W8JWT_EXP_FAILURE){
        printf("Token 已过期。\n");
    } else {
       fprintf(stderr, "error: 其他验证错误，错误码: %u\n", validation_result);   
    }
    return NULL;
}

    Jwt_payload* payload = (Jwt_payload*)malloc(sizeof(Jwt_payload));
    if (payload == NULL) {
        l8w8jwt_free_claims(claims, claims_count);
        return NULL;
    }
    memset(payload, 0, sizeof(Jwt_payload));

    struct l8w8jwt_claim* claim;

    // user_id (sub)
    claim = l8w8jwt_get_claim(claims, claims_count, "sub", strlen("sub"));
    if (claim != NULL && claim->value != NULL) {
        payload->user_id = my_strdup(claim->value);
    } else {
        payload->user_id = NULL;
    }

    // issuer (iss)
    claim = l8w8jwt_get_claim(claims, claims_count, "iss", strlen("iss"));
    if (claim != NULL && claim->value != NULL) {
        payload->issuer = my_strdup(claim->value);
    } else {
        payload->issuer = NULL;
    }

    // audience (aud)
    claim = l8w8jwt_get_claim(claims, claims_count, "aud", strlen("aud"));
    if (claim != NULL && claim->value != NULL) {
        payload->audience = my_strdup(claim->value);
    } else {
        payload->audience = NULL;
    }

    // username (name)
    claim = l8w8jwt_get_claim(claims, claims_count, "name", strlen("name"));
    if (claim != NULL && claim->value != NULL) {
        payload->username = my_strdup(claim->value);
    } else {
        payload->username = NULL;
    }

    // is_admin (字符串 "true"/"false" 转为 int)
    claim = l8w8jwt_get_claim(claims, claims_count, "admin", strlen("admin"));
    if (claim != NULL && claim->value != NULL) {
        if (strcmp(claim->value, "true") == 0 || strcmp(claim->value, "1") == 0) {
            payload->is_admin = 1;
        } else {
            payload->is_admin = 0;
        }
    } else {
        payload->is_admin = 0;
    }

    // iat (时间戳，字符串转数字)
    claim = l8w8jwt_get_claim(claims, claims_count, "iat", strlen("iat"));
    if (claim != NULL && claim->value != NULL) {
        payload->iat = (time_t)atol(claim->value);
    } else {
        payload->iat = 0;
    }

    // exp (时间戳)
    claim = l8w8jwt_get_claim(claims, claims_count, "exp", strlen("exp"));
    if (claim != NULL && claim->value != NULL) {
        payload->exp = (time_t)atol(claim->value);
    } else {
        payload->exp = 0;
    }

    l8w8jwt_free_claims(claims, claims_count);

    return payload;
}

void free_payload(Jwt_payload* payload) {
    if (payload == NULL) {
        return;
    }

    free(payload->user_id);
    free(payload->issuer);
    free(payload->audience);
    free(payload->username);

    // 如果结构体本身是动态分配的，这里也需要释放
    free(payload);
}

void printpayload(Jwt_payload* payload){
 if (payload == NULL) {
        printf("payload 为空\n");
        return;
    }

    printf("用户ID (sub): %s\n", payload->user_id ? payload->user_id : "(null)");
    printf("签发者 (iss): %s\n", payload->issuer ? payload->issuer : "(null)");
    printf("接收者 (aud): %s\n", payload->audience ? payload->audience : "(null)");
    printf("用户名 (name): %s\n", payload->username ? payload->username : "(null)");
    printf("是否管理员 (admin): %s\n", payload->is_admin ? "是" : "否");

    char iat_buf[32];
    char exp_buf[32];

    struct tm* iat_tm = localtime(&(payload->iat));
    struct tm* exp_tm = localtime(&(payload->exp));

    if (iat_tm != NULL) {
        strftime(iat_buf, sizeof(iat_buf), "%Y-%m-%d %H:%M:%S", iat_tm);
        printf("签发时间 (iat): %s\n", iat_buf);
    } else {
        printf("签发时间 (iat): (null)\n");
    }

    if (exp_tm != NULL) {
        strftime(exp_buf, sizeof(exp_buf), "%Y-%m-%d %H:%M:%S", exp_tm);
        printf("过期时间 (exp): %s\n", exp_buf);
    } else {
        printf("过期时间 (exp): (null)\n");
    }

}
