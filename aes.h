#ifndef AES_H
#define AES_H

typedef unsigned char byte;

enum KeySize // key size, in bits, for construtor
{
    Bits128 = 16,
    Bits192 = 24,
    Bits256 = 32
};

class QFile;

class AES
{
public:
    AES();
    AES(int keySize, unsigned char* key);
    ~AES();

private:
    static const unsigned char Sbox[16*16];
    static const unsigned char iSbox[16*16];
    static const unsigned char Rcon[11*4];

    int Nb;
    int Nk;
    int Nr;

    unsigned char key[32];     // the seed key. size will be 4 * keySize from ctor.
    unsigned char w[16*15];      // key schedule array. (Nb*(Nr+1))*4
    unsigned char State[4][4];  // State matrix

private:
    void RunKeyExpansion(int keySize, unsigned char* keyBytes);
    void SetNbNkNr(int keyS);
    void KeyExpansion();
    void AddRoundKey(int round);
    void SubWord(unsigned char * word,unsigned char* result);
    void RotWord(unsigned char * word,unsigned char* result);
    void SubBytes();
    void InvSubBytes();
    void ShiftRows();
    void InvShiftRows();
    void MixColumns();
    void InvMixColumns();
    unsigned char gfmultby01(unsigned char b);
    unsigned char gfmultby02(unsigned char b);
    unsigned char gfmultby03(unsigned char b);
    unsigned char gfmultby09(unsigned char b);
    unsigned char gfmultby0b(unsigned char b);
    unsigned char gfmultby0d(unsigned char b);
    unsigned char gfmultby0e(unsigned char b);

public:
    void FileCipher(QFile *fin, QFile *fout);
    void FileInvCipher(QFile *fin, QFile *fout);

    void Cipher(unsigned char* input, unsigned char* output);
    void InvCipher(unsigned char * input, unsigned char * output);

    void Cipher(unsigned char* input, unsigned char* output,
                unsigned char *key, int keySize = Bits128);
    void InvCipher(unsigned char * input, unsigned char * output,
                   unsigned char *key, int keySize = Bits128);

};

#endif // AES_H
