#ifndef _msg_h_
#define _msg_h_
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "motor.h"
#include "tune.h"
#include "bitcoin.h"
#include "mbed.h"

extern RawSerial pc;

extern Thread getMessageT;
extern Thread receiveMessageT;

typedef struct {
    uint8_t code;
    float data; 
    uint64_t data_64;
}mail_tc;

extern mail_tc mail_t;
extern Mail<mail_tc,8> mail_box;
extern Mail<uint8_t,8> inCharQ;

extern float max_velocity;
extern float max_rotation;

extern bool velocityEnter;
extern bool tuneEnter;

extern Mutex rotation_mutex;
extern Mutex velocity_mutex;
extern Mutex tune_mutex;

extern void getmsg();
extern void receivemsg();
extern void putMessage(uint8_t type, float variable, uint64_t variable_64);
extern void serialISR();

#endif
