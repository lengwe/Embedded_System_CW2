#ifndef _msg_h_
#define _msg_h_
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "motor.h"
#include "tune.h"
#include "bitcoin.h"
#include "mbed.h"


extern Thread thread;
extern Thread receiveMessage;

extern void getmsg();
extern void receivemsg();

#endif