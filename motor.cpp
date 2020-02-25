#include "motor.h"

//Drive state to output table
const int8_t driveTable[] = {0x12,0x18,0x09,0x21,0x24,0x06,0x00,0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07,0x05,0x03,0x04,0x01,0x00,0x02,0x07};  
//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed

//Phase lead to make motor spin
const int32_t PWM_PRD = 2500;
const int8_t lead = 2;  //2 for forwards, -2 for backwards
volatile int8_t orState = 0;    //Rotot offset at motor state 0
volatile int8_t intState = 0;
volatile int8_t intStateOld = 0;

//Initialise the serial port
Serial pc(SERIAL_TX, SERIAL_RX);

//Status LED
DigitalOut led1(LED1);

//Photointerrupter inputs
InterruptIn I1(I1pin);
InterruptIn I2(I2pin);
InterruptIn I3(I3pin);

//Motor Drive outputs
DigitalOut L1L(L1Lpin);
DigitalOut L1H(L1Hpin);
DigitalOut L2L(L2Lpin);
DigitalOut L2H(L2Hpin);
DigitalOut L3L(L3Lpin);
DigitalOut L3H(L3Hpin);

//DigitalOut TP1(TP1pin);
PwmOut MotorPWM(PWMpin);

//Set a given drive state
void motorOut(int8_t driveState){
    
    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];
      
    //Turn off first
    if (~driveOut & 0x01) L1L = 0;
    if (~driveOut & 0x02) L1H = 1;
    if (~driveOut & 0x04) L2L = 0;
    if (~driveOut & 0x08) L2H = 1;
    if (~driveOut & 0x10) L3L = 0;
    if (~driveOut & 0x20) L3H = 1;
    
    //Then turn on
    if (driveOut & 0x01) L1L = 1;
    if (driveOut & 0x02) L1H = 0;
    if (driveOut & 0x04) L2L = 1;
    if (driveOut & 0x08) L2H = 0;
    if (driveOut & 0x10) L3L = 1;
    if (driveOut & 0x20) L3H = 0;
    }
    
    //Convert photointerrupter inputs to a rotor state
inline int8_t readRotorState(){
    return stateMap[I1 + 2*I2 + 4*I3];
    }

//Basic synchronisation routine    
int8_t motorHome() {
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0);
    wait(2.0);
    
    //Get the rotor state
    return readRotorState();
}

void GetSate_interrupt(){    
    intState = stateMap[I1 + 2*I2 + 4*I3];
    motorOut((intState-orState+lead+6)%6); //+6 to make sure the remainder is positive
    }

void ISR(void){
        I1.rise(&GetSate_interrupt);
        I1.fall(&GetSate_interrupt);
        I2.rise(&GetSate_interrupt);
        I2.fall(&GetSate_interrupt); 
        I3.rise(&GetSate_interrupt);
        I3.fall(&GetSate_interrupt);
    }

void setup(){
    MotorPWM.period_us(PWM_PRD);
    MotorPWM.pulsewidth_us(PWM_PRD);
    
    //Initialise the serial port
    //Serial pc(SERIAL_TX, SERIAL_RX);
    pc.printf("Hello\n\r");
    
    //Run the motor synchronisation
    orState = motorHome();
    pc.printf("Rotor origin: %x\n\r",orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states
    
    MotorPWM.pulsewidth_us(PWM_PRD/2);
    //Poll the rotor state and set the motor outputs accordingly to spin the motor
    }