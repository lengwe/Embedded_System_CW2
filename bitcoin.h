#ifndef _bitcoin_h_
#define _bitcoin_h_
#include <string>
#include "mbed.h"
//#include "motor.h"
#include "./Crypto/hash/SHA256.h"

#define NONCE 1
#define COUNT 2
#define KEY 3
#define MAX_VEL 4
#define ROTATE 5
#define ACT_VELOCITY 6
#define MELODY 7
#define ERROR 8

extern RawSerial pc;

//extern uint8_t sequence[];
extern volatile uint64_t* key;
extern uint64_t* nonce;
//extern uint8_t hash[32];
extern volatile uint32_t counter;
extern uint64_t newKey;
extern Mutex newKey_mutex;

typedef struct {
    /*uint32_t count;
    uint64_t nonce;*/
    uint8_t code;
    uint64_t data; 

}mail_tc;

extern mail_tc mail_t;
extern Mail<mail_tc,8> mail_box;
extern Mail<uint8_t,8> inCharQ;


extern void computation();
extern void HashRate(); 
extern void putMessage(uint8_t type, uint64_t variable);
extern void serialISR();

#endif