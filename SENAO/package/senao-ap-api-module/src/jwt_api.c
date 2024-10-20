#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <json.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <jwt_api.h>

/*********Library*************/

/* Macro to allocate a new JWT */
#define ALLOC_JWT(__jwt) do {		\
	int __ret = jwt_new(__jwt);	\
} while(0)
#define JWT_ALG_INVAL JWT_ALG_TERM
#define APPEND_STR(__buf, __str) do {		\
	int ret = __append_str(__buf, __str);	\
	if (ret)				\
		return ret;			\
} while(0)
#define SIGN_ERROR(__err) { ret = __err; goto jwt_sign_sha_pem_done; }
#define VERIFY_ERROR(__err) { ret = __err; goto jwt_verify_sha_pem_done; }
#define ENCODE_ERROR(__err) { ret = __err; goto jwt_encode_error_done; }

#define JSON_INDENT(n)      (n & 0x1F)
#define JSON_COMPACT        0x20
#define JSON_ENSURE_ASCII   0x40
#define JSON_SORT_KEYS      0x80
#define JSON_PRESERVE_ORDER 0x100
#define JSON_ENCODE_ANY     0x200/** Opaque JWT object. */

#define SENAO_JWT_KEY  "ReSVZeE88bn9ZCHKDzepT2zuQ36aaFT"  //31Bytes

static const unsigned char pr2six[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

int jwt_rand_gen(char *output, int size)
{
    FILE *fp = NULL, *fp_jwt_rand = NULL;
    int randno, i;
    const char valid_char[63]="_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    // shell: head /dev/urandom | tr -dc A-Za-z0-9 | head -c 13 ; echo ''

    fp_jwt_rand = fopen(SENAO_JWT_RANDOM_TOKEN_FILE, "r");

    if ( fp_jwt_rand )
    {
        fread(output, size-1, 1, fp_jwt_rand);
        fclose(fp_jwt_rand);
    }
    else
    { // no rand_token exist, create new
        fp = fopen("/dev/urandom", "r");
        if (fp < 0)
        {
            debug_print("/dev/urandom not exist");
            return -1;
        }
        else
        {
            for(i=0; i<size-1; ++i)
            {
                randno = fgetc(fp);
                output[i] = valid_char[randno % sizeof(valid_char)];
            }
        }
        output[i]='\0';

        fclose(fp);

        fp_jwt_rand = fopen(SENAO_JWT_RANDOM_TOKEN_FILE, "w");
        if ( fp_jwt_rand )
        {
            fprintf(fp_jwt_rand, "%s", output);
            fclose(fp_jwt_rand);
        }
    }

    return 0;
}

int jwt_Base64decode(char *bufplain, const char *bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register unsigned char *bufout;
    register int nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);
    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) bufplain;
    bufin = (const unsigned char *) bufcoded;

    while (nprbytes > 4) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    bufin += 4;
    nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}
static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int jwt_Base64encode(char *encoded, const char *string, int len)
{
    int i;
    char *p;

    p = encoded;
    for (i = 0; i < len - 2; i += 3) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    *p++ = basis_64[((string[i] & 0x3) << 4) |
                    ((int) (string[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((string[i + 1] & 0xF) << 2) |
                    ((int) (string[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[string[i + 2] & 0x3F];
    }
    if (i < len) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    if (i == (len - 1)) {
        *p++ = basis_64[((string[i] & 0x3) << 4)];
        *p++ = '=';
    }
    else {
        *p++ = basis_64[((string[i] & 0x3) << 4) |
                        ((int) (string[i + 1] & 0xF0) >> 4)];
        *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
    }

    *p++ = '\0';
    return p - encoded;
}
static int __append_str(char **buf, const char *str)
{
	char *new;

	if (*buf == NULL) {
		new = calloc(1, strlen(str) + 1);
	} else {
		new = realloc(*buf, strlen(*buf) + strlen(str) + 1);
	}

	if (new == NULL)
		return ENOMEM;

	strcat(new, str);

	*buf = new;

	return 0;
}
static const char *get_js_string(json_object *js, const char *key)
{
	const char *val = NULL;
	json_object *js_val;

	js_val = json_object_object_get(js, key);
	if (js_val) {
		val = json_object_get_string(js_val);
	} else {
		errno = ENOENT;
	}

	return val;
}
int jwt_add_grant(jwt_t *jwt, const char *grant, const char *val)
{
	if (!jwt || !grant || !strlen(grant) || !val)
		return EINVAL;

	if (json_object_object_get(jwt->grants, grant) != NULL)
		return EEXIST;

	json_object_object_add(jwt->grants, grant, json_object_new_string(val));

	return 0;
}
#if 0
static long get_js_int(json_t *js, const char *key)
{
	long val = -1;
	json_object *js_val;

	js_val = json_object_object_get(js, key);
	if (js_val) {
		val = (long)json_object_get_int(js_val);
	} else {
		errno = ENOENT;
	}

	return val;
}
int jwt_add_grant_int(jwt_t *jwt, const char *grant, long val)
{
	if (!jwt || !grant || !strlen(grant))
		return EINVAL;

	if (get_js_int(jwt->grants, grant) != -1)
		return EEXIST;

	if (json_object_set_new(jwt->grants, grant, json_integer((json_int_t)val)))
		return EINVAL;

	return 0;
}
#endif
static void jwt_scrub_key(jwt_t *jwt)
{
	if (jwt->key) {
		/* Overwrite it so it's gone from memory. */
		memset(jwt->key, 0, jwt->key_len);

		free(jwt->key);
		jwt->key = NULL;
	}

	jwt->key_len = 0;
	jwt->alg = JWT_ALG_NONE;
}
int jwt_set_alg(jwt_t *jwt, jwt_alg_t alg, const unsigned char *key, int len)
{
	/* No matter what happens here, we do this. */
	jwt_scrub_key(jwt);

	if (alg < JWT_ALG_NONE || alg >= JWT_ALG_INVAL)
		return EINVAL;

	switch (alg) {
	case JWT_ALG_NONE:
		if (key || len)
			return EINVAL;
		break;

	default:
		if (!key || len <= 0)
			return EINVAL;

		jwt->key = malloc(len);
		if (!jwt->key)
			return ENOMEM;

		memcpy(jwt->key, key, len);
	}

	jwt->alg = alg;
	jwt->key_len = len;

	return 0;
}
static int jwt_write_body(jwt_t *jwt, char **buf, int pretty)
{
	/* Sort keys for repeatability */
	size_t flags = JSON_SORT_KEYS;
	char *serial;

	if (pretty) {
		APPEND_STR(buf, "\n");
		flags |= JSON_INDENT(4);
	} else {
		flags |= JSON_COMPACT;
	}

	serial = json_object_to_json_string(jwt->grants);

	APPEND_STR(buf, serial);

	//free(serial);

	if (pretty)
		APPEND_STR(buf, "\n");

	return 0;
}
const char *jwt_alg_str(jwt_alg_t alg)
{
	switch (alg) {
	case JWT_ALG_NONE:
		return "none";
	case JWT_ALG_HS256:
		return "HS256";
	case JWT_ALG_HS384:
		return "HS384";
	case JWT_ALG_HS512:
		return "HS512";
	case JWT_ALG_RS256:
		return "RS256";
	case JWT_ALG_RS384:
		return "RS384";
	case JWT_ALG_RS512:
		return "RS512";
	case JWT_ALG_ES256:
		return "ES256";
	case JWT_ALG_ES384:
		return "ES384";
	case JWT_ALG_ES512:
		return "ES512";
	default:
		return NULL;
	}
}
static int jwt_write_head(jwt_t *jwt, char **buf, int pretty)
{
	APPEND_STR(buf, "{");

	if (pretty)
		APPEND_STR(buf, "\n");

	/* An unsecured JWT is a JWS and provides no "typ".
	 * -- draft-ietf-oauth-json-web-token-32 #6. */
	if (jwt->alg != JWT_ALG_NONE) {
		if (pretty)
			APPEND_STR(buf, "    ");

		APPEND_STR(buf, "\"typ\":");
		if (pretty)
			APPEND_STR(buf, " ");
		APPEND_STR(buf, "\"JWT\",");

		if (pretty)
			APPEND_STR(buf, "\n");
	}

	if (pretty)
		APPEND_STR(buf, "    ");

	APPEND_STR(buf, "\"alg\":");
	if (pretty)
		APPEND_STR(buf, " ");
	APPEND_STR(buf, "\"");
	APPEND_STR(buf, jwt_alg_str(jwt->alg));
	APPEND_STR(buf, "\"");

	if (pretty)
		APPEND_STR(buf, "\n");

	APPEND_STR(buf, "}");

	if (pretty)
		APPEND_STR(buf, "\n");

	return 0;
}
int jwt_sign_sha_hmac(jwt_t *jwt, char **out, unsigned int *len,
		      const char *str)
{
	const EVP_MD *alg;

	switch (jwt->alg) {
        /* HMAC */
	case JWT_ALG_HS256:
		alg = EVP_sha256();
		break;
	case JWT_ALG_HS384:
		alg = EVP_sha384();
		break;
	case JWT_ALG_HS512:
		alg = EVP_sha512();
		break;
	default:
		return EINVAL;
	}

	*out = malloc(EVP_MAX_MD_SIZE);
	if (*out == NULL)
		return ENOMEM;

	HMAC(alg, jwt->key, jwt->key_len,
	     (const unsigned char *)str, strlen(str), (unsigned char *)*out,
	     len);

	return 0;
}
#if OPENSSL_VERSION_NUMBER < 0x10100000L
static void ECDSA_SIG_get0(const ECDSA_SIG *sig, const BIGNUM **pr, const BIGNUM **ps)
{
	if (pr != NULL)
		*pr = sig->r;
	if (ps != NULL)
		*ps = sig->s;
}
#endif
int jwt_sign_sha_pem(jwt_t *jwt, char **out, unsigned int *len,
		     const char *str)
{
	EVP_MD_CTX *mdctx = NULL;
	ECDSA_SIG *ec_sig = NULL;
	const BIGNUM *ec_sig_r = NULL;
	const BIGNUM *ec_sig_s = NULL;
	BIO *bufkey = NULL;
	const EVP_MD *alg;
	int type;
	EVP_PKEY *pkey = NULL;
	int pkey_type;
	unsigned char *sig = NULL;
	int ret = 0;
	size_t slen;
	unsigned char *raw_buf = NULL;

	switch (jwt->alg) {
	/* RSA */
	case JWT_ALG_RS256:
		alg = EVP_sha256();
		type = EVP_PKEY_RSA;
		break;
	case JWT_ALG_RS384:
		alg = EVP_sha384();
		type = EVP_PKEY_RSA;
		break;
	case JWT_ALG_RS512:
		alg = EVP_sha512();
		type = EVP_PKEY_RSA;
		break;

	/* ECC */
	case JWT_ALG_ES256:
		alg = EVP_sha256();
		type = EVP_PKEY_EC;
		break;
	case JWT_ALG_ES384:
		alg = EVP_sha384();
		type = EVP_PKEY_EC;
		break;
	case JWT_ALG_ES512:
		alg = EVP_sha512();
		type = EVP_PKEY_EC;
		break;

	default:
		return EINVAL;
	}

	bufkey = BIO_new_mem_buf(jwt->key, jwt->key_len);
	if (bufkey == NULL)
		SIGN_ERROR(ENOMEM);

	/* This uses OpenSSL's default passphrase callback if needed. The
	 * library caller can override this in many ways, all of which are
	 * outside of the scope of LibJWT and this is documented in jwt.h. */
	pkey = PEM_read_bio_PrivateKey(bufkey, NULL, NULL, NULL);
	if (pkey == NULL)
		SIGN_ERROR(EINVAL);

	pkey_type = EVP_PKEY_id(pkey);
	if (pkey_type != type)
		SIGN_ERROR(EINVAL);

	mdctx = EVP_MD_CTX_create();
	if (mdctx == NULL)
		SIGN_ERROR(ENOMEM);

	/* Initialize the DigestSign operation using alg */
	if (EVP_DigestSignInit(mdctx, NULL, alg, NULL, pkey) != 1)
		SIGN_ERROR(EINVAL);

	/* Call update with the message */
	if (EVP_DigestSignUpdate(mdctx, str, strlen(str)) != 1)
		SIGN_ERROR(EINVAL);

	/* First, call EVP_DigestSignFinal with a NULL sig parameter to get length
	 * of sig. Length is returned in slen */
	if (EVP_DigestSignFinal(mdctx, NULL, &slen) != 1)
		SIGN_ERROR(EINVAL);

	/* Allocate memory for signature based on returned size */
	sig = malloc(slen);
	if (sig == NULL)
		SIGN_ERROR(ENOMEM);

	/* Get the signature */
	if (EVP_DigestSignFinal(mdctx, sig, &slen) != 1)
		SIGN_ERROR(EINVAL);

	if (pkey_type != EVP_PKEY_EC) {
		*out = malloc(slen);
		if (*out == NULL)
			SIGN_ERROR(ENOMEM);
		memcpy(*out, sig, slen);
		*len = slen;
	} else {
		unsigned int degree, bn_len, r_len, s_len, buf_len;
		EC_KEY *ec_key;

		/* For EC we need to convert to a raw format of R/S. */

		/* Get the actual ec_key */
		ec_key = EVP_PKEY_get1_EC_KEY(pkey);
		if (ec_key == NULL)
			SIGN_ERROR(ENOMEM);

		degree = EC_GROUP_get_degree(EC_KEY_get0_group(ec_key));

		EC_KEY_free(ec_key);

		/* Get the sig from the DER encoded version. */
		ec_sig = d2i_ECDSA_SIG(NULL, (const unsigned char **)&sig, slen);
		if (ec_sig == NULL)
			SIGN_ERROR(ENOMEM);

		ECDSA_SIG_get0(ec_sig, &ec_sig_r, &ec_sig_s);
		r_len = BN_num_bytes(ec_sig_r);
		s_len = BN_num_bytes(ec_sig_s);
		bn_len = (degree + 7) / 8;
		if ((r_len > bn_len) || (s_len > bn_len))
			SIGN_ERROR(EINVAL);

		buf_len = 2 * bn_len;
		raw_buf = malloc(buf_len);
		if (raw_buf == NULL)
			SIGN_ERROR(ENOMEM);

		/* Pad the bignums with leading zeroes. */
		memset(raw_buf, 0, buf_len);
		BN_bn2bin(ec_sig_r, raw_buf + bn_len - r_len);
		BN_bn2bin(ec_sig_s, raw_buf + buf_len - s_len);

		*out = malloc(buf_len);
		if (*out == NULL)
			SIGN_ERROR(ENOMEM);
		memcpy(*out, raw_buf, buf_len);
		*len = buf_len;
	}

jwt_sign_sha_pem_done:
	if (bufkey)
		BIO_free(bufkey);
	if (pkey)
		EVP_PKEY_free(pkey);
	if (mdctx)
		EVP_MD_CTX_destroy(mdctx);
	if (ec_sig)
		ECDSA_SIG_free(ec_sig);

	free(raw_buf);
	free(sig);

	return ret;
}
static int jwt_sign(jwt_t *jwt, char **out, unsigned int *len, const char *str)
{
	switch (jwt->alg) {
	/* HMAC */
	case JWT_ALG_HS256:
	case JWT_ALG_HS384:
	case JWT_ALG_HS512:
		return jwt_sign_sha_hmac(jwt, out, len, str);

	/* RSA */
	case JWT_ALG_RS256:
	case JWT_ALG_RS384:
	case JWT_ALG_RS512:

	/* ECC */
	case JWT_ALG_ES256:
	case JWT_ALG_ES384:
	case JWT_ALG_ES512:
		return jwt_sign_sha_pem(jwt, out, len, str);

	/* You wut, mate? */
	default:
		return EINVAL;
	}
}
void jwt_base64uri_encode(char *str)
{
	int len = strlen(str);
	int i, t;

	for (i = t = 0; i < len; i++) {
		switch (str[i]) {
		case '+':
			str[t++] = '-';
			break;
		case '/':
			str[t++] = '_';
			break;
		case '=':
			break;
		default:
			str[t++] = str[i];
		}
	}

	str[t] = '\0';
}
static int jwt_encode(jwt_t *jwt, char **out)
{
	char *buf = NULL, *head = NULL, *body = NULL, *sig = NULL;
	int ret, head_len, body_len, res;
	unsigned int sig_len;

	/* First the header. */
	res = jwt_write_head(jwt, &buf, 0);
	if (res) {
		ENCODE_ERROR(res);
	}

	head = malloc(strlen(buf) * 2);
	if (head == NULL) {
		free(buf);
		return ENOMEM;
	}
	jwt_Base64encode(head, buf, strlen(buf));
	head_len = strlen(head);

	free(buf);
	buf = NULL;

	/* Now the body. */
	res = jwt_write_body(jwt, &buf, 0);
	if (res) {
		ENCODE_ERROR(res);
	}

	body = malloc(strlen(buf) * 2);
	if (body == NULL) {
		ENCODE_ERROR(ENOMEM);
	}
	jwt_Base64encode(body, buf, strlen(buf));
	body_len = strlen(body);

	free(buf);
	buf = NULL;

	jwt_base64uri_encode(head);
	jwt_base64uri_encode(body);

	/* Allocate enough to reuse as b64 buffer. */
	buf = malloc(head_len + body_len + 2);
	if (buf == NULL)
	{
		ENCODE_ERROR(ENOMEM);
    }
	strcpy(buf, head);
	strcat(buf, ".");
	strcat(buf, body);

	res = __append_str(out, buf);
	if (res == 0)
		res = __append_str(out, ".");
	if (res) {
		ENCODE_ERROR(res);
	}

	if (jwt->alg == JWT_ALG_NONE) {
		ENCODE_ERROR(0);
	}

	/* Now the signature. */
	res = jwt_sign(jwt, &sig, &sig_len, buf);
	free(buf);
	buf = NULL;

	if (res)
	{
		ENCODE_ERROR(res);
	}

	buf = malloc(sig_len * 2);
	if (buf == NULL) {
		ENCODE_ERROR(ENOMEM);
	}

	jwt_Base64encode(buf, sig, sig_len);


	jwt_base64uri_encode(buf);
	ret = __append_str(out, buf);

jwt_encode_error_done:
	free(sig);
	free(body);
	free(head);
	free(buf);

	return ret;
}
char *jwt_encode_str(jwt_t *jwt)
{
	char *str = NULL;

	errno = jwt_encode(jwt, &str);
	if (errno) {
		if (str)
			free(str);
		str = NULL;
	}

	return str;
}
void jwt_free(jwt_t *jwt)
{
	if (!jwt)
		return;

	jwt_scrub_key(jwt);

	if (jwt->grants) {
		json_object_put(jwt->grants);
		jwt->grants = NULL;
	}

	free(jwt);
}
const char *jwt_get_grant(jwt_t *jwt, const char *grant)
{
	if (!jwt || !grant || !strlen(grant)) {
		errno = EINVAL;
		return NULL;
	}

	errno = 0;

	return get_js_string(jwt->grants, grant);
}
int jwt_new(jwt_t **jwt)
{
	if (!jwt)
		return EINVAL;

	*jwt = malloc(sizeof(jwt_t));
	if (!*jwt)
		return ENOMEM;

	memset(*jwt, 0, sizeof(jwt_t));


	(*jwt)->grants = json_object_new_object();
	if (!(*jwt)->grants) {
		free(*jwt);
		*jwt = NULL;
		return ENOMEM;
	}

	return 0;
}
void *jwt_b64_decode(const char *src, int *ret_len)
{
	void *buf = NULL;
	char *new = NULL;
	int len, i, z;

	/* Decode based on RFC-4648 URI safe encoding. */
	len = strlen(src);
	new = malloc(len + 4);
	if (!new)
		return NULL;

	for (i = 0; i < len; i++) {
		switch (src[i]) {
		case '-':
			new[i] = '+';
			break;
		case '_':
			new[i] = '/';
			break;
		default:
			new[i] = src[i];
		}
	}
	z = 4 - (i % 4);
	if (z < 4) {
		while (z--)
			new[i++] = '=';
	}
	new[i] = '\0';

	buf = malloc(i);
	if (buf == NULL)
	{
		free(new);
		return NULL;
	}

	*ret_len = jwt_Base64decode(buf, new);
	free(new);

	return buf;
}
static json_object *jwt_b64_decode_json(char *src)
{
	json_object *js;
	char *buf;
	int len;

	buf = jwt_b64_decode(src, &len);

	if (buf == NULL)
		return NULL;

	buf[len] = '\0';

	js = json_tokener_parse(buf);

	free(buf);

	return js;
}
jwt_alg_t jwt_str_alg(const char *alg)
{
	if (alg == NULL)
		return JWT_ALG_INVAL;

	if (!strcasecmp(alg, "none"))
		return JWT_ALG_NONE;
	else if (!strcasecmp(alg, "HS256"))
		return JWT_ALG_HS256;
	else if (!strcasecmp(alg, "HS384"))
		return JWT_ALG_HS384;
	else if (!strcasecmp(alg, "HS512"))
		return JWT_ALG_HS512;
	else if (!strcasecmp(alg, "RS256"))
		return JWT_ALG_RS256;
	else if (!strcasecmp(alg, "RS384"))
		return JWT_ALG_RS384;
	else if (!strcasecmp(alg, "RS512"))
		return JWT_ALG_RS512;
	else if (!strcasecmp(alg, "ES256"))
		return JWT_ALG_ES256;
	else if (!strcasecmp(alg, "ES384"))
		return JWT_ALG_ES384;
	else if (!strcasecmp(alg, "ES512"))
		return JWT_ALG_ES512;

	return JWT_ALG_INVAL;
}

int senao_json_object_get_string(struct json_object *jobj, char *obj_name, char *res)
{
    struct json_object *jobj_str;
    int result;

    result = FALSE;

    if((jobj_str = json_object_object_get(jobj, obj_name)))
    {
        sprintf(res, "%s", json_object_get_string(jobj_str));

        /* Get the setting successfully. */
        result = TRUE;

        /* Free obj */
        json_object_put(jobj_str);
    }

    return result;
}


static int jwt_verify_head(jwt_t *jwt, char *head)
{
	json_object *js = NULL;
	const char *val;
	int ret = 0;

	js = jwt_b64_decode_json(head);
	if (!js)
		return EINVAL;

	val = get_js_string(js, "alg");
	jwt->alg = jwt_str_alg(val);
	if (jwt->alg == JWT_ALG_INVAL) {
		ret = EINVAL;
		goto verify_head_done;
	}

	if (jwt->alg != JWT_ALG_NONE) {
		/* If alg is not NONE, there may be a typ. */
		val = get_js_string(js, "typ");
		if (val && strcasecmp(val, "JWT"))
			ret = EINVAL;

		if (jwt->key) {
			if (jwt->key_len <= 0)
				ret = EINVAL;
		} else {
			jwt_scrub_key(jwt);
		}
	} else {
		/* If alg is NONE, there should not be a key */
		if (jwt->key){
			ret = EINVAL;
		}
	}

verify_head_done:
	if (js)
		json_object_put(js);

	return ret;
}
int jwt_verify_sha_hmac(jwt_t *jwt, const char *head, const char *sig)
{
	unsigned char res[EVP_MAX_MD_SIZE];
	BIO *bmem = NULL, *b64 = NULL;
	unsigned int res_len;
	const EVP_MD *alg;
	char *buf = NULL;
	int len, ret = EINVAL;

	switch (jwt->alg) {
	case JWT_ALG_HS256:
		alg = EVP_sha256();
		break;
	case JWT_ALG_HS384:
		alg = EVP_sha384();
		break;
	case JWT_ALG_HS512:
		alg = EVP_sha512();
		break;
	default:
		return EINVAL;
	}

	b64 = BIO_new(BIO_f_base64());
	if (b64 == NULL)
		return ENOMEM;

	bmem = BIO_new(BIO_s_mem());
	if (bmem == NULL) {
		BIO_free(b64);
		return ENOMEM;
	}

	BIO_push(b64, bmem);
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	HMAC(alg, jwt->key, jwt->key_len,
	     (const unsigned char *)head, strlen(head), res, &res_len);

	BIO_write(b64, res, res_len);

	(void)BIO_flush(b64);

	len = BIO_pending(bmem);
	if (len < 0)
		goto jwt_verify_hmac_done;

	buf = malloc(len + 1);
	if (!buf) {
		ret = ENOMEM;
		goto jwt_verify_hmac_done;
	}

	len = BIO_read(bmem, buf, len);
	buf[len] = '\0';

	jwt_base64uri_encode(buf);

	/* And now... */
	ret = strcmp(buf, sig) ? EINVAL : 0;

jwt_verify_hmac_done:
	BIO_free_all(b64);
	free(buf);

	return ret;
}
#if OPENSSL_VERSION_NUMBER < 0x10100000L
static int ECDSA_SIG_set0(ECDSA_SIG *sig, BIGNUM *r, BIGNUM *s)
{
	if (r == NULL || s == NULL)
		return 0;

	BN_clear_free(sig->r);
	BN_clear_free(sig->s);
	sig->r = r;
	sig->s = s;

	return 1;
}
#endif
int jwt_verify_sha_pem(jwt_t *jwt, const char *head, const char *sig_b64)
{
	unsigned char *sig = NULL;
	EVP_MD_CTX *mdctx = NULL;
	ECDSA_SIG *ec_sig = NULL;
	BIGNUM *ec_sig_r = NULL;
	BIGNUM *ec_sig_s = NULL;
	EVP_PKEY *pkey = NULL;
	const EVP_MD *alg;
	int type;
	int pkey_type;
	BIO *bufkey = NULL;
	int ret = 0;
	int slen;

	switch (jwt->alg) {
	/* RSA */
	case JWT_ALG_RS256:
		alg = EVP_sha256();
		type = EVP_PKEY_RSA;
		break;
	case JWT_ALG_RS384:
		alg = EVP_sha384();
		type = EVP_PKEY_RSA;
		break;
	case JWT_ALG_RS512:
		alg = EVP_sha512();
		type = EVP_PKEY_RSA;
		break;

	/* ECC */
	case JWT_ALG_ES256:
		alg = EVP_sha256();
		type = EVP_PKEY_EC;
		break;
	case JWT_ALG_ES384:
		alg = EVP_sha384();
		type = EVP_PKEY_EC;
		break;
	case JWT_ALG_ES512:
		alg = EVP_sha512();
		type = EVP_PKEY_EC;
		break;

	default:
		return EINVAL;
	}

	sig = jwt_b64_decode(sig_b64, &slen);
	if (sig == NULL)
		VERIFY_ERROR(EINVAL);

	bufkey = BIO_new_mem_buf(jwt->key, jwt->key_len);
	if (bufkey == NULL)
		VERIFY_ERROR(ENOMEM);

	/* This uses OpenSSL's default passphrase callback if needed. The
	 * library caller can override this in many ways, all of which are
	 * outside of the scope of LibJWT and this is documented in jwt.h. */
	pkey = PEM_read_bio_PUBKEY(bufkey, NULL, NULL, NULL);
	if (pkey == NULL)
		VERIFY_ERROR(EINVAL);

	pkey_type = EVP_PKEY_id(pkey);
	if (pkey_type != type)
		VERIFY_ERROR(EINVAL);

	/* Convert EC sigs back to ASN1. */
	if (pkey_type == EVP_PKEY_EC) {
		unsigned int degree, bn_len;
		unsigned char *p;
		EC_KEY *ec_key;

		ec_sig = ECDSA_SIG_new();
		if (ec_sig == NULL)
			VERIFY_ERROR(ENOMEM);

		/* Get the actual ec_key */
		ec_key = EVP_PKEY_get1_EC_KEY(pkey);
		if (ec_key == NULL)
			VERIFY_ERROR(ENOMEM);

		degree = EC_GROUP_get_degree(EC_KEY_get0_group(ec_key));

		EC_KEY_free(ec_key);

		bn_len = (degree + 7) / 8;
		if ((bn_len * 2) != slen)
			VERIFY_ERROR(EINVAL);

		ec_sig_r = BN_bin2bn(sig, bn_len, NULL);
		ec_sig_s = BN_bin2bn(sig + bn_len, bn_len, NULL);
		if (ec_sig_r  == NULL || ec_sig_s == NULL)
			VERIFY_ERROR(EINVAL);

		ECDSA_SIG_set0(ec_sig, ec_sig_r, ec_sig_s);
		free(sig);

		slen = i2d_ECDSA_SIG(ec_sig, NULL);
		sig = malloc(slen);
		if (sig == NULL)
			VERIFY_ERROR(ENOMEM);

		p = sig;
		slen = i2d_ECDSA_SIG(ec_sig, &p);

		if (slen == 0)
			VERIFY_ERROR(EINVAL);
	}

	mdctx = EVP_MD_CTX_create();
	if (mdctx == NULL)
		VERIFY_ERROR(ENOMEM);

	/* Initialize the DigestVerify operation using alg */
	if (EVP_DigestVerifyInit(mdctx, NULL, alg, NULL, pkey) != 1)
		VERIFY_ERROR(EINVAL);

	/* Call update with the message */
	if (EVP_DigestVerifyUpdate(mdctx, head, strlen(head)) != 1)
		VERIFY_ERROR(EINVAL);

	/* Now check the sig for validity. */
	if (EVP_DigestVerifyFinal(mdctx, sig, slen) != 1)
		VERIFY_ERROR(EINVAL);

jwt_verify_sha_pem_done:
	if (bufkey)
		BIO_free(bufkey);
	if (pkey)
		EVP_PKEY_free(pkey);
	if (mdctx)
		EVP_MD_CTX_destroy(mdctx);
	if (sig)
		free(sig);
	if (ec_sig)
		ECDSA_SIG_free(ec_sig);

	return ret;
}
static int jwt_verify(jwt_t *jwt, const char *head, const char *sig)
{
	switch (jwt->alg) {
	/* HMAC */
	case JWT_ALG_HS256:
	case JWT_ALG_HS384:
	case JWT_ALG_HS512:
		return jwt_verify_sha_hmac(jwt, head, sig);

	/* RSA */
	case JWT_ALG_RS256:
	case JWT_ALG_RS384:
	case JWT_ALG_RS512:

	/* ECC */
	case JWT_ALG_ES256:
	case JWT_ALG_ES384:
	case JWT_ALG_ES512:
		return jwt_verify_sha_pem(jwt, head, sig);

	/* You wut, mate? */
	default:
		return EINVAL;
	}
}
static int jwt_parse_body(jwt_t *jwt, char *body)
{
	if (jwt->grants) {
		json_object_put(jwt->grants);
		jwt->grants = NULL;
	}

	jwt->grants = jwt_b64_decode_json(body);
	if (!jwt->grants)
		return EINVAL;

	return 0;
}
int jwt_decode(jwt_t **jwt, const char *token, const unsigned char *key,
	       int key_len)
{
	char *head = strdup(token);
	jwt_t *new = NULL;
	char *body, *sig;
	int ret = EINVAL;

	if (!jwt)
		return EINVAL;

	*jwt = NULL;

	if (!head)
		return ENOMEM;

	/* Find the components. */
	for (body = head; body[0] != '.'; body++) {
		if (body[0] == '\0')
			goto decode_done;
	}

	body[0] = '\0';
	body++;

	for (sig = body; sig[0] != '.'; sig++) {
		if (sig[0] == '\0')
			goto decode_done;
	}

	sig[0] = '\0';
	sig++;

	/* Now that we have everything split up, let's check out the
	 * header. */
	ret = jwt_new(&new);
	if (ret) {
		goto decode_done;
	}

	/* Copy the key over for verify_head. */
	if (key_len) {
		new->key = malloc(key_len);
		if (new->key == NULL)
			goto decode_done;
		memcpy(new->key, key, key_len);
		new->key_len = key_len;
	}

	ret = jwt_verify_head(new, head);
	if (ret)
		goto decode_done;

	ret = jwt_parse_body(new, body);
	if (ret)
		goto decode_done;

	/* Check the signature, if needed. */
	if (new->alg != JWT_ALG_NONE) {
		/* Re-add this since it's part of the verified data. */
		body[-1] = '.';
		ret = jwt_verify(new, head, sig);
	} else {
		ret = 0;
	}

decode_done:
	if (ret)
		jwt_free(new);
	else
		*jwt = new;

	free(head);

	return ret;
}
/*********Library End*************/

/********** Encode **********/
void encode_hs256(char *role, char **result)
{
    jwt_t *jwt = NULL;
	int ret=0;
    int x=0;
	char *out;
    char *JwtName[3] = {"expr","role","rand"};
    char xtime[32]={0};
    char jwt_rand[17]={0};
    const unsigned char *key = SENAO_JWT_KEY;

    //Set expr time = now + 1hr
    time_t now;
    now = time(NULL);
    x = now + 3600;
    snprintf(xtime, sizeof(xtime), "%d", x);
    jwt_rand_gen(jwt_rand, sizeof(jwt_rand));

    ALLOC_JWT(&jwt);

    jwt_add_grant(jwt, JwtName[0], xtime);
    jwt_add_grant(jwt, JwtName[1], role);
    jwt_add_grant(jwt, JwtName[2], jwt_rand);
    ret = jwt_set_alg(jwt, JWT_ALG_HS256, key, strlen(key));//Set an algorithm from jwt_alg_t for this JWT object.
    if (ret==0){
        debug_print("\n---- Pass !!! ----\n\n");
        *result = jwt_encode_str(jwt);
        debug_print("The Token is:\n[%s]\n\n",*result);
    }
    else{
        debug_print("---- Error!!! ----\n");
    }
    jwt_free(jwt);//Free a JWT object and any other resources it is using.
    return;
}
/********** Encode End **********/ 

/********** Decode **********/
void decode_hs256(char token[], int option, char **result)
{
	jwt_t *jwt = NULL;
    //const char res[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJleHByIjoiMTIzNDAwMDAiLCJuYW1lIjoiYWRtaW4ifQ._iwxOou-hagijJ8T5OM2t-y_18ZzyBM30VrfCRpbV-k";
    const unsigned char *key = SENAO_JWT_KEY;
	int ret = 0;
	char* val  = NULL;
    //ret = jwt_decode(&jwt, res, key, sizeof(key));//Verify an existing JWT and allocate a new JWT object from it.
	ret = jwt_decode(&jwt, token, key, strlen(key));
	debug_print("Jason DEBUG %s[%d], option[%d].\n", __FUNCTION__, __LINE__, option);
	if (ret==0)
    {
        if(option==1) //GET role
        {
            val = jwt_get_grant(jwt, "role");//Return the value of a string grant.
        }
        else if(option==0) //GET expr 
        {
            val = jwt_get_grant(jwt, "expr");//Return the value of a string grant.
        }
        else if ( option == 2 ) // GET rand
        {
            val = jwt_get_grant(jwt, "rand");//Return the value of a string grant.
        }

        if (val)
        {
            *result = strdup(val);
            debug_print("Jason DEBUG %s[%d], role[%s].\n", __FUNCTION__, __LINE__, *result);
        }
        else
            *result = NULL;
    }
    else{
         debug_print("\n---- Error!!! Can't finish decoding !!! ----\n\n");
    }
	jwt_free(jwt);//Free a JWT object and any other resources it is using.
	return;
}
/********** Decode End **********/    
/*
int main()
{
    encode_hs256();
    decode_hs256();
	return 0;
}
*/
