/*
 * hash.c
 *
 *  Created on: 2015-1-9
 *      Author: lovenix
 */

#include "../inc/main.h"
static unsigned long HashString( char *lpszFileName, unsigned int dwHashType);
unsigned int cryptTable[0x500];
void CryptTables()
{
    unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;
	unsigned long temp1, temp2;
    for(index1 = 0; index1 < 0x100; index1++)
    {
        for(index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
        {
            seed = (seed * 125 + 3) % 0x2AAAAB;
            temp1 = (seed & 0xFFFF) << 0x10;
            seed = (seed * 125 + 3) % 0x2AAAAB;
            temp2 = (seed & 0xFFFF);
            cryptTable[index2] = (temp1 | temp2);
        }
    }
}

unsigned long HashString(char *lpszFileName, unsigned int dwHashType)
{
    unsigned char *key  = (unsigned char *)lpszFileName;
    unsigned long seed1 = 0x7FED7FED;
    unsigned long seed2 = 0xEEEEEEEE;
    int ch;
    while(*key != 0)
    {
        ch = toupper(*key++);
        seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
        seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
    }
    return seed1;
}

struct HashItem* inithashtable(int size)
{
    int i;
    struct HashItem* newhashtable=(struct HashItem*)malloc(sizeof(struct HashItem)*size);
    for (i=0; i<size ;i++ )
        newhashtable[i].bExists=0;
    return newhashtable;
}

int FreeHash(struct HashItem* hash)
{
	free(hash);
	return 1;
}

int InsertHash(char *lpszString, struct HashItem *lpTable, unsigned int nTableSize)
{
    int HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;
    unsigned long nHash = HashString(lpszString, HASH_OFFSET);
    unsigned long nHashA = HashString(lpszString, HASH_A);
    unsigned long nHashB = HashString(lpszString, HASH_B);
    unsigned long nHashStart = nHash % nTableSize;
    unsigned long nHashPos = nHashStart;
    while (lpTable[nHashPos].bExists)
    {
        nHashPos = (nHashPos + 1) % nTableSize;
        if (nHashPos == nHashStart)
            break;
    }
    lpTable[nHashPos].bExists=1;
    lpTable[nHashPos].nHashA=nHashA;
    lpTable[nHashPos].nHashB=nHashB;
	return nHashPos;
}

int GetHashTablePos(char *lpszString,struct HashItem* lpTable, unsigned int nTableSize)
{
    int HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;
    unsigned long nHash = HashString(lpszString, HASH_OFFSET);
    unsigned long nHashA = HashString(lpszString, HASH_A);
    unsigned long nHashB = HashString(lpszString, HASH_B);
    unsigned long nHashStart = nHash % nTableSize;
	unsigned long nHashPos = nHashStart;
    while (lpTable[nHashPos].bExists)
    {
        if (lpTable[nHashPos].nHashA == nHashA && lpTable[nHashPos].nHashB == nHashB)
            return nHashPos;
        else
            nHashPos = (nHashPos + 1) % nTableSize;
        if (nHashPos == nHashStart)
            break;
    }
    return 0;
}

int DelHashTablePos(char *lpszString,struct HashItem* lpTable, unsigned int nTableSize)
{
    int HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;
    unsigned int nHash = HashString(lpszString, HASH_OFFSET);
    unsigned int nHashA = HashString(lpszString, HASH_A);
    unsigned int nHashB = HashString(lpszString, HASH_B);
    unsigned int nHashStart = nHash % nTableSize, nHashPos = nHashStart;

    while (lpTable[nHashPos].bExists)
    {
        if (lpTable[nHashPos].nHashA == nHashA && lpTable[nHashPos].nHashB == nHashB)
		{    
			lpTable[nHashPos].bExists=0;
			lpTable[nHashPos].nHashA=0;
			lpTable[nHashPos].nHashB=0;
            return nHashPos;
		}
        else
            nHashPos = (nHashPos + 1) % nTableSize;
        if (nHashPos == nHashStart)
            break;
    }
    return 0;
}
