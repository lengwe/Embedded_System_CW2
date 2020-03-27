#include "msg.h"

Thread thread;
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
//                pc.printf("Nonce is: 0x%llx\n\r",mail->data);
                pc.printf("N%016llX\n\r",mail->data_64);
                break;
            case(COUNT):
                pc.printf("Hash rate is: %d\n\r",mail->data_64);
                break;
            case(MAX_VEL):
                pc.printf("Max Velocity is %f\n\r",mail->data);
                break;
            case(ROTATE):
                  pc.printf("Target Rotation is: %f\n\r",mail->data);
                break;
            case(ACT_VELOCITY):
                pc.printf("Actual velocity is %f\n\r", mail->data);
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
    float max_velocity;
    
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
                    putMessage(KEY, 0, newKey);
                    break;
                case 'V':
                    sscanf(command,"V%f",&max_velocity,0);
                    max_vel = max_velocity;
//                    pc.printf("max_vel is %d \n\r", max_velocity);
                    putMessage(MAX_VEL, max_velocity,0);
                    break;
                case 'R':
                    rotationEnter=true;
                    sscanf(command, "R%f", &rotation);
                    putMessage(ROTATE, rotation,0);
                    break;
                case 'T':
                    sscanf(command,"T%s", tune);
                    note_extraction();
//                    putMessage(ROTATE, (uint64_t)tune);
                    break;
                }
                i=0;
        }
        else{
            i=i+1;
        }
    }
}
