#include "mbed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "motor.h"
#include "bitcoin.h"
#include "tune.h"
#include "msg.h"

int main(){
    setup();
    ISR();
    
    Ticker tick;
//    thread.start(callback(putMessage);
    tick.attach(&HashRate, 1.0);
    //tick will interrupt it to call HashRate
    thread.start(getmsg);
    receiveMessage.start(receivemsg);
    motorCtrlT.start(callback(motorCtrlFn));
    melody.start(callback(playMelody));
    
    while(1){
        
        computation(); 
          
    }
    
}
