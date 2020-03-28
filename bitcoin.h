#ifndef _bitcoin_h_
#define _bitcoin_h_
#include <string>
#include "mbed.h"
#include "msg.h"
//#include "motor.h"
#include "./Crypto/hash/SHA256.h"

#define NONCE 1
#define COUNT 2
#define KEY 3
#define MAX_VEL 4
#define ROTATE 5
#define ACT_VELOCITY 6
#define ERROR 7

//extern uint8_t sequence[];
extern volatile uint64_t* key;
extern uint64_t* nonce;
//extern uint8_t hash[32];
extern volatile uint32_t counter;
extern uint64_t newKey;
extern Mutex newKey_mutex;


extern void computation();
extern void HashRate(); 



#endif
