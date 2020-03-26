#include "motor.h"

//Drive state to output table
const int8_t driveTable[] = {0x12,0x18,0x09,0x21,0x24,0x06,0x00,0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07,0x05,0x03,0x04,0x01,0x00,0x02,0x07};  
//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed

//Phase lead to make motor spin
const int32_t PWM_PRD = 2500;
int8_t lead = -2;  //2 for forwards, -2 for backwards
volatile int8_t orState = 0;    //Rotot offset at motor state 0
volatile int8_t intState = 0;
volatile int8_t intStateOld = 0;
volatile int32_t position = 0;

volatile int32_t speedController;
volatile float yr = 0.0;

volatile float velocity;
float max_vel=100;
float rotation;
float sign;

int prevPosition = 0;
int startPosition = 0;

//error variables
float position_err = 0;
float oldPosition_err = 0;
float speed_err = 0;

float position_tar = 0;
float prevRotation = 0;
float kpr = 0.005;
float kdr = 0.011;
float kps = 0.07;
float kis = 0.0004;
float v_const = 0.013;
float integral_speed_err = 0.0;
float integral_position_err = 0.0;

int32_t old_position_error= 0;
int32_t position_error;
volatile float ys = 0.0;
volatile float y = 0.0;

float diff_position_err = 0.0;
float maxPWM = 1.0;
float inter_err = 0.0;

volatile bool rotationEnter = false;

Thread motorCtrlT (osPriorityNormal,1024);
Timer t;
//Initialise the serial port
//Serial pc(SERIAL_TX, SERIAL_RX);

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

DigitalOut TP1(TP1pin);
PwmOut MotorPWM(PWMpin);
//PwmOut ControlPWM(pin D9);

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
    

//    MotorPWM.write(0.5f);
      MotorPWM.write(y);
//      MotorPWM.period(0.002f);
//    MotorPWM.pulsewidth_us(1000);
    }
    
    //Convert photointerrupter inputs to a rotor state
inline int8_t readRotorState(){
    return stateMap[I1 + 2*I2 + 4*I3];
    }

//Basic synchronisation routine    
int8_t motorHome() {
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0);
    thread_sleep_for(2.0);
    //Get the rotor state
    return readRotorState();
}

void GetSate_interrupt(){    
    intState = stateMap[I1 + 2*I2 + 4*I3];
    motorOut((intState-orState+lead+6)%6); //+6 to make sure the remainder is positive
    
//    pc.printf("intStateOld: %d, intState: %d \n\r", intStateOld, intState);
    if(intState > intStateOld || (intState == 0 && intStateOld == 5)){
        position = position - 1;
    } else {
        position = position + 1;
    }
    intStateOld = intState;
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
    MotorPWM.period_ms(2);
//    MotorPWM.pulsewidth_us(PWM_PRD);

    
    //Initialise the serial port
    //Serial pc(SERIAL_TX, SERIAL_RX);
    pc.printf("Hello\n\r");
    
    //Run the motor synchronisation
    orState = motorHome();
    //motorOut((readRotorState()+lead+6)%6);
    intStateOld = readRotorState();
    pc.printf("Rotor origin: %x\n\r",orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states

//    MotorPWM.pulsewidth_us(PWM_PRD);
    //Poll the rotor state and set the motor outputs accordingly to spin the motor
    }
    
void motorCtrlTick(){
    motorCtrlT.signal_set(0x1);
}

float VelocityControl(){
    
    /*float sign = (velocity>=0)? 1 : -1;
    //float velocity_error = sign*(float)max_vel-(float)velocity;
    float velocity_error = (float)max_vel-(float)abs(velocity);
    inter_err = inter_err + kis*velocity_error/0.1;
    if(inter_err > 880){
        inter_err = 880;
    }
    if(inter_err < -880){
        inter_err = -880;
    }
    
    //ys = kps*velocity_error + inter_err;
    diff_position_err = 10*(float)(position_err - oldPosition_err);
    speed_err = velocity*sign - max_vel;
    
    integral_speed_err = integral_speed_err + speed_err/0.1;
    if(integral_speed_err > 880){
        integral_speed_err = 880;
    }
    if(integral_speed_err < -880){
        integral_speed_err = -880;
    }
    
    float sign_speed_err = (diff_position_err >=0)? -1 : 1;
    
    ys = kps*speed_err + kis*integral_speed_err;
    ys = ys * sign_speed_err;
    lead = (ys<0) ? -2 : 2;
    ys= abs(ys);*/
    
    //////////////////////fucking easy version/////////////////////////
    sign = (velocity>=0)? 1 : -1;
    speed_err = velocity*sign - max_vel;
    integral_speed_err = integral_speed_err + speed_err/0.1;
    if(integral_speed_err > 880){
        integral_speed_err = 880;
    }
    if(integral_speed_err < -880){
        integral_speed_err = -880;
    }
    
    ys = kps*(speed_err)+kis*integral_speed_err;
    lead = (ys<0) ? -lead : lead;
    ys = (ys<0) ? -ys:0;
    ys = (ys > maxPWM) ? maxPWM : ys;
    //pc.printf("ys %f, max_vel %f, yr %f, velocity %f, tar_rotations %f, position err %F\n\r", ys, max_vel, yr, velocity, rotation, position_err);
    ///////////////////////////////////////////////
    return ys;
}

float RotationControl(){
    float yr;

    if(rotationEnter){
        startPosition = position;
        position_tar = abs(rotation) *6;
        rotationEnter = false;
    }
    
    lead = (rotation < 0) ? -2 : 2;
    position_err = position_tar - (float)abs(position - startPosition);
    diff_position_err = (float)(position_err - oldPosition_err);
    //diff_position_err = (diff_position_err<0) ? -diff_position_err:diff_position_err;
    oldPosition_err = position_err;
    yr = kpr * position_err + kdr*diff_position_err-v_const*(float)abs((int)velocity);
    pc.printf("lead: %d ",lead);
    lead = (yr < 0) ? -lead : lead;
    yr = (yr>=0) ? yr : -yr;
//    yr = abs((int)yr);
    yr = (yr > maxPWM) ? maxPWM : yr;
    
    pc.printf("yr %f, tar_ro %f, max_v %f, velocity %f, ys %f, l: %d, position err %F\n\r",yr, rotation, max_vel, velocity, ys,lead, position_err);
    return yr;
}
 
void motorCtrlFn(){
    float v;
    float r;
    motorHome();
    Ticker motorCtrlTicker;
//    motorCtrlTicker.attach_us(&motorCtrlTick, 500000);

    t.start();
    int startTime = t.read_us();
    int counter=0;
    while(1){

        if (t.read_us() - startTime >= 100000) {
            // store the position from which start to count the number of rotations.
            
            
            // wait for signal to occur
//            motorCtrlT.signal_wait(0x1);
            
//            pc.printf("%f",position_tar);
            velocity = ((float)position - (float)prevPosition)*10.0/6.0;
//            pc.printf("actual velocity is %f, prevPosition is %d, position %d\n\r", velocity, prevPosition, position);
            // if rotations or max velocity is set and the motor is stopped, start rotating.
            if(counter == 9){
                putMessage(ACT_VELOCITY,velocity);
                counter = 0;
                }
            else{
                counter = counter + 1;
            }
                
            
//            if(velocity == 0 && rotationEnter){
//                  motorOut((readRotorState()-orState+lead+6)%6);
////                motorOut(readRotorState());
////                MotorPWM.wr?ite(1.0f);
//                
//            }
            
    //        pc.printf("position %d, prevPosition %d, velocity %f\n\r", position, prevPosition, velocity);

            /*diff_position_err = (float)(position_err - oldPosition_err);
            speed_err = (float)abs((int)diff_position_err)-(int)max_vel;
            integral_speed_err = integral_speed_err + speed_err;
            
            ys = kps*speed_err + kis*integral_speed_err;
            
            if (ys > 2000) {
                ys = 2000;
            }
            else if (ys < -2000){
                ys = -2000;
            }
            
            // adjust the lead according to the sign of ys
            
//            ys = (diff_position_err < 0) ? -ys : ys;
            lead = (ys < 0) ? -2 : 2;
            // check if ys exceed maximum value set
            
            yr = kpr * position_err + kdr*diff_position_err;
            yr = (yr < 0) ? 0 : yr;
            
            ys = (ys < 0) ? 0 : ys;
            y = (diff_position_err < 0) ? MAX(ys, yr): MIN(ys, yr);*/ 
            if (rotation == 0 && max_vel != 0){
                y = VelocityControl();
            } else if (rotation != 0 && max_vel != 0){
                v = VelocityControl();
                r = RotationControl();
                motorOut((readRotorState()-orState+lead+6)%6);                
//                y = ((max_vel>30)&&((sign*velocity) < max_vel) && (position_err > 50)) ? MAX(v, r): MIN(v, r);
                
                if(max_vel>30){
                     y = (((sign*velocity) < 18) && (position_err > 30)) ? MAX(v, r): MIN(v, r);
                    }
                else{
                     y = (((sign*velocity) < max_vel/2) && (position_err > 30)) ? MAX(v, r): MIN(v, r);
                    }
//                y = MIN(v, r);
            
               
            }
//            MotorPWM.write(y);
//            pc.printf("max_vel:%F \n\r",max_vel);
            //MotorPWM.write(y);
//            pc.printf("ys %f, max_vel %f, yr %f, velocity %f, tar_rotations %f, position err %F\n\r", ys, max_vel, yr, velocity, rotation, position_err);
            startTime = t.read_us();
            prevPosition = position;
        }
    }
}
