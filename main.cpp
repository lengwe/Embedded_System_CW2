#include "mbed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "motor.h"
#include "bitcoin.h"
#include "tune.h"
#include "msg.h"

/*Thread thread;
Thread receiveMessage;
//Thread motorCtrlT (osPriorityNormal,1024);
char command[18];
uint64_t receivedKey;
int i = 0;
int j = 0;


void getmsg(){
    while(1){
    osEvent evt = mail_box.get();
    if(evt.status == osEventMail){
        mail_tc *mail = (mail_tc*)evt.value.p;
        switch(mail->code){
            case(NONCE):
                //pc.printf("Nonce is: 0x%llx\n\r",mail->data);
                break;
            case(COUNT):
               // pc.printf("Hash rate is: %llu\n\r",mail->data);
                break;
            case(MAX_VEL):
//                max_vel = mail->data;
                //pc.printf("Velocity is %d",max_vel);
                break;
            case(ROTATE):
                rotation = (float)mail->data;
                break;
            case(ACT_VELOCITY):
               // pc.printf("actual velocity is %f\n\r", mail->data);
                break;
            case(MELODY):
//              pc.printf("")
                break;
            case(ERROR):
//                pc.printf("ERROR!!!");
                break;
            }
        mail_box.free(mail);
        }        
    }
    
}


void receivemsg(){ 
//    string s;
    uint16_t hex;
    int max_velocity;
    
    pc.attach(&serialISR);
    while(1){
        osEvent newEvent = inCharQ.get();
        uint8_t* newChar = (uint8_t*)newEvent.value.p;
        
        command[i] = *newChar;
        inCharQ.free(newChar);
        //i+=1;
        if(command[i]=='\r'){
//            pc.printf("in!");
            wait_us(100);
//            pc.printf("hex2 is %x",hex);

            command[i+1] = '\0' ;
            switch(command[0]){
                case 'K':
                    //pc.printf("K");
//                    for(j = 0; j<i;j++){
//                        pc.printf("%c",command[j]);
//                        }
                    sscanf(command,"K%hx",&hex);
//                    pc.printf("hex is %x \n\r",hex);
                    receivedKey = (uint64_t)hex;
                    newKey_mutex.lock();
                    newKey = receivedKey;
                    newKey_mutex.unlock();
                    putMessage(KEY, newKey);
                    break;
                case 'V':
                    sscanf(command,"V%d",&max_velocity);
                    max_vel = max_velocity;
//                    pc.printf("max_vel is %d \n\r", max_velocity);
                    putMessage(MAX_VEL, (uint64_t) max_velocity);
                    break;
                case 'R':
                    if(command[1] == '-') {
                        sscanf(command, "R-%f", &rotation);
                        rotation = -rotation;
                    }
                    else {
                        sscanf(command, "R%f", &rotation);
                    }
                    putMessage(ROTATE, (uint64_t)rotation);
                    break;
                case 'T':
                    sscanf(command,"T%s", tune);
                    putMessage(ROTATE, (uint64_t)tune);
                    break;
                }
                i=0;
        }
        else{
            i=i+1;
        }
    }
}*/


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
