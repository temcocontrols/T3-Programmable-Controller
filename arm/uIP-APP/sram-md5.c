// cram-md5.cpp : ??????????????
//
 
#include "md5.h"
#include <string.h>
 #include "main.h"
 
/////////////////
 
#define CRAM_MD5_CONTEXTLEN 32
 
//#define MD5_RESULTLEN 16  // tbd:???????
 
struct hmac_md5_context {
    MD5_CTX ctx;
		MD5_CTX ctxo;
};
 
void hmac_md5_init(struct hmac_md5_context *ctx,
    const unsigned char *key, size_t key_len);
//void hmac_md5_final(struct hmac_md5_context *ctx,unsigned char digest[MD5_RESULTLEN]);
 
//void hmac_md5_get_cram_context(struct hmac_md5_context *ctx,
//    unsigned char context_digest[CRAM_MD5_CONTEXTLEN]);
//void hmac_md5_set_cram_context(struct hmac_md5_context *ctx,
//    const unsigned char context_digest[CRAM_MD5_CONTEXTLEN]);
 
void hmac_md5_get_cram_context(struct hmac_md5_context *ctx,
    unsigned char *context_digest);
void hmac_md5_set_cram_context(struct hmac_md5_context *ctx,
     unsigned char *context_digest);


// change data tyep by chelsea
static void hmac_md5_update(struct hmac_md5_context *ctx, unsigned char *dat, size_t size)
//hmac_md5_update(struct hmac_md5_context *ctx, const void *dat, size_t size)
{
    MD5Update(&ctx->ctx, dat, size);
}
 
void hmac_md5_init(struct hmac_md5_context *ctx,
    const unsigned char *key, size_t key_len)
{
    int i;
    unsigned char md5key[16];
    unsigned char k_ipad[64];
    unsigned char k_opad[64];
 
// delete by chelsea, do not have md5_get_digest() ?????????
//    if (key_len > 64) {
//        md5_get_digest(key, key_len, md5key);
//        key = md5key;
//        key_len = 16;
//    }
 
    memcpy(k_ipad, key, key_len);
    memset(k_ipad + key_len, 0, 64 - key_len);
    memcpy(k_opad, k_ipad, 64);
 
    for (i = 0; i < 64; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }
 
    MD5Init(&ctx->ctx);
    MD5Update(&ctx->ctx, k_ipad, 64);
    MD5Init(&ctx->ctxo);
    MD5Update(&ctx->ctxo, k_opad, 64);
 
    memset(k_ipad, 0, 64);
    memset(k_opad, 0, 64);
}
 
void hmac_md5_final(struct hmac_md5_context *ctx, unsigned char *digest)
{
    MD5Final(&ctx->ctx, digest);
 
    MD5Update(&ctx->ctxo, digest, 16);
    MD5Final(&ctx->ctxo, digest);
}
 
#define CDPUT(p, c) {   \
    *(p)++ = (c)& 0xff;       \
    *(p)++ = (c) >> 8 & 0xff;  \
    *(p)++ = (c) >> 16 & 0xff; \
    *(p)++ = (c) >> 24 & 0xff; \
    }
void hmac_md5_get_cram_context(struct hmac_md5_context *ctx,
    //unsigned char context_digest[CRAM_MD5_CONTEXTLEN])
			unsigned char * context_digest)
{
    unsigned char *cdp;
 
//#define CDPUT(p, c) {   \
//    *(p)++ = (c)& 0xff;       \
//    *(p)++ = (c) >> 8 & 0xff;  \
//    *(p)++ = (c) >> 16 & 0xff; \
//    *(p)++ = (c) >> 24 & 0xff; \
//    }
//    cdp = context_digest;
//    CDPUT(cdp, ctx->ctxo.a);
//    CDPUT(cdp, ctx->ctxo.b);
//    CDPUT(cdp, ctx->ctxo.c);
//    CDPUT(cdp, ctx->ctxo.d);
//    CDPUT(cdp, ctx->ctx.a);
//    CDPUT(cdp, ctx->ctx.b);
//    CDPUT(cdp, ctx->ctx.c);
//    CDPUT(cdp, ctx->ctx.d);
	
		cdp = context_digest;
    CDPUT(cdp, ctx->ctxo.state[0]);
    CDPUT(cdp, ctx->ctxo.state[1]);
    CDPUT(cdp, ctx->ctxo.state[2]);
    CDPUT(cdp, ctx->ctxo.state[3]);
    CDPUT(cdp, ctx->ctx.state[0]);
    CDPUT(cdp, ctx->ctx.state[1]);
    CDPUT(cdp, ctx->ctx.state[2]);
    CDPUT(cdp, ctx->ctx.state[3]);
}
 
#define CDGET(p, c) { \
    (c) = (*p++);           \
    (c) += (*p++ << 8);      \
    (c) += (*p++ << 16);     \
    (c) += (*p++ << 24);     \
    }
void hmac_md5_set_cram_context(struct hmac_md5_context *ctx,
    //const unsigned char context_digest[CRAM_MD5_CONTEXTLEN])
			unsigned char * context_digest)
{
    const unsigned char *cdp;
 
//#define CDGET(p, c) { \
//    (c) = (*p++);           \
//    (c) += (*p++ << 8);      \
//    (c) += (*p++ << 16);     \
//    (c) += (*p++ << 24);     \
//    }
//    cdp = context_digest;
//    CDGET(cdp, ctx->ctxo.a);
//    CDGET(cdp, ctx->ctxo.b);
//    CDGET(cdp, ctx->ctxo.c);
//    CDGET(cdp, ctx->ctxo.d);
//    CDGET(cdp, ctx->ctx.a);
//    CDGET(cdp, ctx->ctx.b);
//    CDGET(cdp, ctx->ctx.c);
//    CDGET(cdp, ctx->ctx.d);
// 
//    ctx->ctxo.lo = ctx->ctx.lo = 64;
//    ctx->ctxo.hi = ctx->ctx.hi = 0;
	
    cdp = context_digest;
    CDGET(cdp, ctx->ctxo.state[0]);
    CDGET(cdp, ctx->ctxo.state[1]);
    CDGET(cdp, ctx->ctxo.state[2]);
    CDGET(cdp, ctx->ctxo.state[3]);
    CDGET(cdp, ctx->ctx.state[0]);
    CDGET(cdp, ctx->ctx.state[1]);
    CDGET(cdp, ctx->ctx.state[2]);
    CDGET(cdp, ctx->ctx.state[3]);
 
    ctx->ctxo.count[0] = ctx->ctx.count[0] = 64;
    ctx->ctxo.count[1] = ctx->ctx.count[1] = 0;
}
 
//int _tmain(int argc, _TCHAR* argv[])
//{
//    unsigned char hmac[CRAM_MD5_CONTEXTLEN] = { 0 };
// 
//    hmac_md5_context ctx;
//    hmac_md5_init(&ctx, (const unsigned char *)"123", 3);
//    hmac_md5_get_cram_context(&ctx, hmac);
// 
//    return 0;
//}

void cram_md5(char *s,unsigned char *cram)
{
//	unsigned char hmac[CRAM_MD5_CONTEXTLEN] = { 0 };
	int i;
	struct hmac_md5_context ctx;
	hmac_md5_init(&ctx, s, strlen(s));
	hmac_md5_get_cram_context(&ctx, cram);
}

void test_scram(void)
{
		unsigned char hmac[CRAM_MD5_CONTEXTLEN] = { 0 };
		int i;
    struct hmac_md5_context ctx;
    hmac_md5_init(&ctx, (const unsigned char *)"user:chelsea pass:jsrtcaw@#dD1", 3);
    hmac_md5_get_cram_context(&ctx, hmac);
		
#if 1//ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
//	printf("rcv %u, %d %d %d %d %d %d %d %d %d %d\r\n",length,pbuf[0],pbuf[1],
//		pbuf[2],pbuf[3],pbuf[4],pbuf[5],pbuf[6],pbuf[7],pbuf[8],pbuf[9]);
		for(i=0;i<32;i++)
	{
		printf("%02x",hmac[i]);
	}
#endif
}