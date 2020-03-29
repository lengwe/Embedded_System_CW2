#include "tune.h"
#include <stdio.h>
#include <stdlib.h>

char tune[49] = {0};
float seconds[16];
float freq[12] = {261.6256,277.1826,293.6648,311.1270,329.6276,349.2282,369.9944,391.9954,415.3047,440.0000,466.1638,493.8833};
int index[17] = {-1};
Mutex pwm_lock;


//Thread melody (osPriorityNormal,1024);

void note_extraction(){
    int i = 0;
    int j = 0;
    int m = 0;
    memset(index, -1 , sizeof(index));
    
    tune_mutex.lock();
    while(tune[i]!=0){
        switch(tune[i]){
            case 'A':
                index[j] = A;
//                pc.printf("A%d",index[j]);
                break;
            case 'B':
                index[j] = B;
//                pc.printf("B%d",index[j]);
                break;     
            case 'C':
                index[j] = C;
//                pc.printf("C%d",index[j]);
                break;
            case 'D':
                index[j] = D;
//                pc.printf("D%d",index[j]);
                break;  
            case 'E':
                index[j] = E;
//                pc.printf("E%d",index[j]);
                break;  
            case 'F':
                index[j] = F;
//                pc.printf("F%d",index[j]);
                break;    
            case 'G':
                index[j] = G; 
//                pc.printf("G%d",index[j]);  
                break;
            case '#':
                index[j]+=1;
                break;
            case '^':
                index[j]-=1;
                break;
            default:
//                pc.printf("index: %d\r\n",index[i]);
                if(tune[i]>='1'&&tune[i]<='8'){
                    seconds[j]=(float)tune[i]-48;
//                    pc.printf("%F\n\r",seconds[j]);
                    j+=1;   
                }
                else{
                    putMessage(ERROR,0,0);    
                }
                break;
        }
        i+=1;
    }  
    tune_mutex.unlock();  
    memset(tune, 0, sizeof(tune));
}

void playMelody(){
    while(1){
        int k=0;
        ThisThread::sleep_for(1);
        
        tune_mutex.lock();
        while(index[k]!=-1){
            float duration = 1.0/seconds[k];
            float time_period = 1.0/freq[index[k]];
            
            MotorPWM.period(time_period);
            ThisThread::sleep_for(duration*1000.0);
            k+=1;
        }
        tune_mutex.unlock();
    }
}
