#ifndef _bitcoin_h_
#define _bitcoin_h_

#include "mbed.h"
#include "./motor/motor.h"
#include "./Crypto/hash/SHA256.h"

//extern uint8_t sequence[];
//extern uint64_t* key;
//extern uint64_t* nonce;
//extern uint8_t hash[32];
//extern volatile uint32_t counter;

extern void computation();
extern void HashRate(); 

#endif