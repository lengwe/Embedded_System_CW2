#include "msg.h"

RawSerial pc(SERIAL_TX, SERIAL_RX);

char command[18];
uint64_t receivedKey;
float max_velocity;
float max_rotation;

bool velocityEnter = false;
    
int i = 0;
int j = 0;

Mail<mail_tc,8> mail_box;
Mail<uint8_t,8> inCharQ;

Mutex rotation_mutex;
Mutex velocity_mutex;
Mutex tune_mutex;


void getmsg(){
    while(1){
    osEvent evt = mail_box.get();
    if(evt.status == osEventMail){
        mail_tc *mail = (mail_tc*)evt.value.p;
        switch(mail->code){
            case(NONCE):
                pc.printf("N%016llX\n\r",mail->data_64);
                break;
            case(COUNT):
                pc.printf("Hash rate is: %d\n\r",mail->data_64);
                break;
            case(KEY):
                pc.printf("K%016llX\n\r",mail->data_64);
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
//            case 8:
//                pc.printf("Pos_err is %f\n\r", mail->data);
//                break;
            case(ERROR):
                pc.printf("ERROR!!!");
                break;
            }
        mail_box.free(mail);
        }        
    }
    
}

void serialISR(){
    uint8_t* newChar = inCharQ.alloc();
    *newChar = pc.getc();
    inCharQ.put(newChar);
}

void putMessage(uint8_t type, float variable, uint64_t variable_64){
    mail_tc *mail = mail_box.alloc();
    mail->code = type;
    mail->data = variable; 
    mail->data_64 = variable_64;
    mail_box.put(mail);
}


void receivemsg(){ 
//    string s;
    uint16_t hex;
    
    pc.attach(&serialISR);
    while(1){
        osEvent newEvent = inCharQ.get();
        uint8_t* newChar = (uint8_t*)newEvent.value.p;
        
        command[i] = *newChar;
        inCharQ.free(newChar);
        //i+=1;
        if(command[i]=='\r'){
            wait_us(100);

            command[i+1] = '\0' ;
            switch(command[0]){
                case 'K':
                    sscanf(command,"K%hx",&hex);
                    receivedKey = (uint64_t)hex;
                    newKey_mutex.lock();
                    newKey = receivedKey;
                    newKey_mutex.unlock();
                    putMessage(KEY, 0, newKey);
                    break;
                case 'V':
                    velocityEnter=true;
                    velocity_mutex.lock();
                    sscanf(command,"V%f",&max_velocity);
                    velocity_mutex.unlock();
                    putMessage(MAX_VEL, max_velocity,0);
                    break;
                case 'R':
                    rotationEnter=true;
                    rotation_mutex.lock();
                    sscanf(command, "R%f", &max_rotation);
                    rotation_mutex.unlock();
                    putMessage(ROTATE, max_rotation,0);
                    break;
                case 'T':
                    sscanf(command,"T%s", tune);
                    note_extraction();
                    break;
                }
                i=0;
        }
        else{
            i=i+1;
        }
    }
}
