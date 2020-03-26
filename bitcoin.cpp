#include "bitcoin.h"

RawSerial pc(SERIAL_TX, SERIAL_RX);

volatile uint32_t counter = 0;
uint8_t sequence[] = {0x45,0x6D,0x62,0x65,0x64,0x64,0x65,0x64,
                0x20,0x53,0x79,0x73,0x74,0x65,0x6D,0x73,
                0x20,0x61,0x72,0x65,0x20,0x66,0x75,0x6E,
                0x20,0x61,0x6E,0x64,0x20,0x64,0x6F,0x20,
                0x61,0x77,0x65,0x73,0x6F,0x6D,0x65,0x20,
                0x74,0x68,0x69,0x6E,0x67,0x73,0x21,0x20,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
                
volatile uint64_t* key = (uint64_t*)&sequence[48];
uint64_t* nonce = (uint64_t*)&sequence[56];
uint64_t newKey;
Mutex newKey_mutex;
//typedef struct {
//    uint32_t counter;
//    //uint64_t nonce;
//
//}mail_t;
Mail<mail_tc,8> mail_box;
Mail<uint8_t,8> inCharQ;

void serialISR(){
    uint8_t* newChar = inCharQ.alloc();
    *newChar = pc.getc();
    inCharQ.put(newChar);
}

void putMessage(uint8_t type, double variable){
    mail_tc *mail = mail_box.alloc();
    mail->code = type;
    mail->data = variable; 
    mail_box.put(mail);
}

void computation(){
    uint8_t hash2[32];
//    newKey_mutex.lock();
//    *key = newKey;
//    newKey_mutex.unlock();
    SHA256::computeHash(hash2,sequence,64);
    if(hash2[0]==0 && hash2[1]==0){
//          pc.printf("Nonce is: 0x%x\r\n",*nonce);
        putMessage(NONCE, (double)*nonce);
    }
    *nonce = *nonce + 1;
    counter +=1;
}

void HashRate(){
        putMessage(COUNT, (uint64_t)counter);
        counter=0;
    }
    
