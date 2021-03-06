/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#define KEYLENGTH(keybits) ((keybits)/8)
#define RKLENGTH(keybits)  ((keybits)/8+28)
#define NROUNDS(keybits)   ((keybits)/32+6)

enum
{
	KEYBITS = 256,
	CRYPTO_BUFFER_SIZE = 16,
	SHA256_BLOCK_LENGTH = 64,
	SHA256_DIGEST_LENGTH = 32,
	WP_DIGEST_SIZE = 64,
	PBKDF2_OUTPUT_LENGTH = 32,
	
	AES_DECRYPT = false,
	AES_ENCRYPT = true,
	
	AES_CBC = false,
	AES_ECB = true
};

/*****************************************************
**                                                  **
**                      Rijndael                    **
**                                                  **
*****************************************************/

typedef uint32_t RK_KEY;

int rijndaelSetupEncrypt(RK_KEY *rk, const unsigned char *key, int keybits);
int rijndaelSetupDecrypt(RK_KEY *rk, const unsigned char *key, int keybits);
void rijndaelEncrypt(const RK_KEY *rk, int nrounds, const rawData plaintext[16], rawData ciphertext[16]);
void rijndaelDecrypt(const RK_KEY *rk, int nrounds, const rawData ciphertext[16], rawData plaintext[16]);

//Helpers
void _AES(const void *_password, const void *_pathInput, uint lengthInput, void *_pathOutput, int cryptIntoMemory, bool encrypt, bool ECB);
void AESEncrypt(const void *_password, const void *_path_input, void *_path_output, int cryptIntoMemory);
void AESDecrypt(const void *_password, const void *_path_input, void *_path_output, int cryptIntoMemory);

/*****************************************************
**                                                  **
**                      Serpent                     **
**                                                  **
*****************************************************/

typedef struct
{
	uint32_t key[140];
} SerpentInstance;

void serpent_set_key(const uint8_t userKey[], int keylen, SerpentInstance *ks);
void serpent_encrypt(SerpentInstance *ks, const uint8_t *inBlock, uint8_t *outBlock);
void serpent_decrypt(SerpentInstance *ks, const uint8_t *inBlock, uint8_t *outBlock);

/*****************************************************
**                                                  **
**                      Twofish                     **
**                                                  **
*****************************************************/

typedef struct
{
	uint32_t l_key[40];
	uint32_t s_key[4];
	uint32_t mk_tab[4 * 256];
	uint32_t k_len;

} TwofishInstance;

//Truecrypt don't use the return value, so I guess it's fine to discard it
uint32_t * TwofishSetKey(TwofishInstance *instance, const uint32_t in_key[], const uint32_t key_len);
void TwofishEncrypt(TwofishInstance *instance, const uint32_t in_blk[4], uint32_t out_blk[]);
void TwofishDecrypt(TwofishInstance *instance, const uint32_t in_blk[4], uint32_t out_blk[4]);

/*****************************************************
**                                                  **
**                       CRC32                      **
**                                                  **
*****************************************************/

uint32_t _crc32(const void *buf, size_t size);
uint32_t crc32File(char * filename);

/*****************************************************
**                                                  **
**                      PBKDF2                      **
**                                                  **
*****************************************************/

int internal_pbkdf2(uint32_t prf_hlen, const uint8_t *input, size_t inputLength, const uint8_t *salt, size_t saltLength, uint32_t iteneration, size_t lengthOutput, uint8_t *output);
void pbkdf2(uint8_t input[], uint8_t salt[], uint8_t output[]);


/*****************************************************
**                                                  **
**                      SHA-256                     **
**                                                  **
*****************************************************/

int sha256(unsigned char* input, void* output);
int sha256_legacy(const char input[], char output[2*SHA256_DIGEST_LENGTH+1]);
void sha256_salted(const uint8_t *input, size_t inputLen, const uint8_t *salt, size_t saltlen, uint8_t *output);

/*****************************************************
**                                                  **
**                    Whirlpool                     **
**                                                  **
*****************************************************/

void whirlpool(const unsigned char *input, size_t inputLength, char *output, bool outHex);

/*****************************************************
 **                                                  **
 **                  More stuffs                     **
 **                                                  **
 *****************************************************/

typedef struct
{
	rawData * bufIn;
	rawData * bufOut;
	size_t length;
	rawData CBC[2][CRYPTO_BUFFER_SIZE];
	
	atomic_int * decWhenJobDone;

	TwofishInstance twofish;
	SerpentInstance serpent;
	
	byte depth;
	bool needFreeMemory;
	
} DECRYPT_PAGE_DATA;
