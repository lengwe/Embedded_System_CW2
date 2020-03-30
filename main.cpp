#include "mbed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "motor.h"
#include "bitcoin.h"
#include "tune.h"
#include "msg.h"


Thread receiveMessage(osPriorityNormal,1024);
Thread getMessage(osPriorityNormal,1024);
Thread melody(osPriorityNormal,1024);
Thread motorCtrlT (osPriorityHigh,1024);

int main(){
    setup();
    ISR();
    
    Ticker tick;
    
    getMessage.start(getmsg);
    receiveMessage.start(receivemsg);
    motorCtrlT.start(motorCtrlFn);
    melody.start(playMelody);

    tick.attach(&HashRate, 1.0);
 
    while(1){
        computation(); 
    }
}
