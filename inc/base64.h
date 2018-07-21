#ifndef BASE64_H_
#define BASE64_H_
extern int base64Encoder(const unsigned char *src, char *enc, int sLen);
extern int base64Decode(const char *enc, unsigned char *dec);
#endif
