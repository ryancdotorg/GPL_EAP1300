#ifndef _JWT_API_H_
#define _JWT_API_H_

/** JWT algorithm types. */
typedef enum jwt_alg {
    JWT_ALG_NONE = 0,
    JWT_ALG_HS256,
    JWT_ALG_HS384,
    JWT_ALG_HS512,
    JWT_ALG_RS256,
    JWT_ALG_RS384,
    JWT_ALG_RS512,
    JWT_ALG_ES256,
    JWT_ALG_ES384,
    JWT_ALG_ES512,
    JWT_ALG_TERM
} jwt_alg_t;
/* encoding */

#define SENAO_JWT_RANDOM_TOKEN_FILE "/tmp/jwt_random"

typedef struct jwt jwt_t;
struct jwt {
    jwt_alg_t alg;
    unsigned char *key;
    int key_len;
    json_object *grants;
};

#endif
