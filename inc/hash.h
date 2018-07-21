/*********** hash.h ***************/
#ifndef HASH_H_
#define HASH_H_

extern void  CryptTables(); 							//
extern struct HashItem* inithashtable(int size);
extern int InsertHash(char *lpszString, struct HashItem *lpTable, unsigned int nTableSize);
extern int GetHashTablePos(char *lpszString, struct HashItem* lpTable,unsigned int nTableSize); //
extern int DelHashTablePos(char *lpszString, struct HashItem* lpTable,unsigned int nTableSize); //
extern int FreeHash(struct HashItem* hash);
struct HashItem{
	int bExists;
	unsigned long nHashA;
	unsigned long nHashB;
};
extern struct HashItem* RidHash;
extern struct HashItem* WidHash;
extern struct HashItem* SmsHash;
#endif
