#include "mbed.h"
#include "motor.h"
#include "bitcoin.h"

int main(){
    setup();
    ISR();
    
    Ticker tick;
    tick.attach(&HashRate,1)
    //tick will interrupt it to call HashRate
    
    while(1){
        computation();       
    }
    
}