#include "../inc/main.h"
const char base64[65] = {
    65,66,67,68,69,70,71,72,73,74,
    75,76,77,78,79,80,81,82,83,84,
    85,86,87,88,89,90,97,98,99,100,
    101,102,103,104,105,106,107,108,109,110,
    111,112,113,114,115,116,117,118,119,120,
    121,122,48,49,50,51,52,53,54,55,
    56,57,43,47,61
};

int base64Encoder(const unsigned char *src, char *enc, int sLen)
{
    int i, j;
    unsigned char current;
    for (i = 0,j = 0; i<sLen; i+=3)
    {
        current = (src[i] >> 2) ;
        current &= (unsigned char)0x3F;
        enc[j++] = base64[(int)current];

        current = ((unsigned char)(src[i] << 4)) & ((unsigned char)0x30) ;
        if (i + 1 >= sLen)
        {
            enc[j++] = base64[(int)current];
            enc[j++] = '=';
            enc[j++] = '=';
            break;
        }
        current |= ((unsigned char)(src[i+1] >> 4)) & ((unsigned char) 0x0F);
        enc[j++] = base64[(int)current];

        current = ((unsigned char)(src[i+1] << 2)) & ((unsigned char)0x3C) ;
        if (i + 2 >= sLen)
        {
            enc[j++] = base64[(int)current];
            enc[j++] = '=';
            break;
        }
        current |= ((unsigned char)(src[i+2] >> 6)) & ((unsigned char) 0x03);
        enc[j++] = base64[(int)current];

        current = ((unsigned char)src[i+2]) & ((unsigned char)0x3F) ;
        enc[j++] = base64[(int)current];
    }
    enc[j] = '\0';
    return 1;
}

int base64Decode(const char *enc, unsigned char *dec)
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for (i=0, j=0; enc[i] != '\0' ; i+=4)
    {
        memset( temp, 0xFF, sizeof(temp));
        for(k=0; k<64 ; k++)
        {
            if (base64[k] == enc[i])
                temp[0]= k;
        }
        for(k=0; k<64; k++)
        {
            if (base64[k] == enc[i+1])
                temp[1]= k;
        }
        for(k=0; k<64; k++)
        {
            if (base64[k] == enc[i+2])
                temp[2]= k;
        }
        for(k=0; k<64; k++)
        {
            if (base64[k] == enc[i+3])
                temp[3]= k;
        }

        dec[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) | ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if (enc[i+2] == '=')
            break;

        dec[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) | ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if (enc[i+3] == '=')
            break;
        dec[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) | ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}
