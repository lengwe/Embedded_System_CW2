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

volatile float velocity;
float max_vel=100.0;
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
float kdr = 0.033;
//float kps = 0.013;
//float kis = 0.0006;
float kps = 0.55;
float kis = 0.0006;
float v_const = 0.013;
float integral_speed_err = 0.0;
float integral_position_err = 0.0;

int32_t old_position_error= 0;
int32_t position_error;
volatile float ys = 0.0;
volatile float y = 1.0;
volatile float yr = 0.0;

float diff_position_err = 0.0;
float maxPWM = 1.0;
float inter_err = 0.0;

float average_vel = 0.0;

volatile bool rotationEnter = false;

//Thread motorCtrlT (osPriorityHigh,1024);
Timer t;

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

    MotorPWM.write(y);
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
//    MotorPWM.write(y);
    
//    pc.printf("intStateOld: %d, intState: %d \n\r", intStateOld, intState);
    if(intState > intStateOld || (intState == 0 && intStateOld == 5)){
        position = position + 1;
    } else {
        position = position - 1;
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
//    MotorPWM.pulsewidth_us(PWM_PRD);
    MotorPWM.write(1.0f);
    
    //Initialise the serial port
    pc.printf("Hello\n\r");
    
    //Run the motor synchronisation
    orState = motorHome();
    y = 0.0;
    intStateOld = readRotorState();
    pc.printf("Rotor origin: %x\n\r",orState);
}
    
void motorCtrlTick(){
    motorCtrlT.flags_set(0x1);
}

float VelocityControl(){
    
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
    ys = (ys<0) ? -ys:0;
    ys = (ys > maxPWM) ? maxPWM : ys;
    //pc.printf("ys %f, max_vel %f, yr %f, velocity %f, tar_rotations %f, position err %F\n\r", ys, max_vel, yr, velocity, rotation, position_err);
    return ys;
}

float RotationControl(){
    float yr;
    
    lead = (rotation < 0) ? -2 : 2;
    position_err = position_tar - (float)abs(position - startPosition);
    diff_position_err = (float)(position_err - oldPosition_err);
    oldPosition_err = position_err;
    
    yr = kpr * position_err + kdr*diff_position_err-v_const*(float)abs((int)velocity);
    lead = (yr < 0) ? -lead : lead;
    yr = (yr>=0) ? yr : -yr;
    yr = (yr > maxPWM) ? maxPWM : yr;
    
//    pc.printf("yr %f, tar_ro %f, max_v %f, velocity %f, ys %f, l: %d, position err %F\n\r",yr, rotation, max_vel, velocity, ys,lead, position_err);
    return yr;
}
 
void motorCtrlFn(){
    float v;
    float r;

    Ticker motorCtrlTicker;
    motorCtrlTicker.attach_us(&motorCtrlTick, 100000);
    
    float tv = 0.0;
    t.start();
    int startTime = t.read_us();
    int counter=0;
    while(1){
//        if (t.read_us() - startTime  >= 100000) {
            ThisThread::flags_wait_all(0x1);
            t.stop();
            tv = t.read();
            t.reset();
            t.start();
            
            velocity = ((float)position - (float)prevPosition)/(tv*6.0);
            average_vel += velocity;
//            pc.printf("actual velocity is %f, prevPosition is %d, position %d\n\r", velocity, prevPosition, position);
            // if rotations or max velocity is set and the motor is stopped, start rotating.
            if(counter == 9){
                average_vel = average_vel/10;
//                putMessage(ACT_VELOCITY,position_err,0);
                  putMessage(ACT_VELOCITY,average_vel,0);
                  putMessage(8,position_err,0);
                average_vel = 0.0;
                counter = 0;
            }
            else{
                counter = counter + 1;
            }

            if(velocityEnter==true){
                velocity_mutex.lock();
                max_vel = max_velocity;
                velocity_mutex.unlock();
                velocityEnter=false;
            }
            
            // store the position from which start to count the number of rotations.
            else if(rotationEnter){
                rotation_mutex.lock();
                rotation = max_rotation;
                rotation_mutex.unlock();
                startPosition = position;
                position_tar = abs(rotation)*6;
                rotationEnter = false;
            }
            
            if (rotation != 0 && max_vel != 0){
                v = VelocityControl();
                r = RotationControl();
                motorOut((readRotorState()-orState+lead+6)%6);
            
                if(max_vel>30){
                     y = (((sign*velocity) < 18) && (position_err >= 4)) ? MAX(v, r): MIN(v, r);
                    }
                else{
                     y = (((sign*velocity) < max_vel/2) && (position_err >= 4)) ? MAX(v, r): MIN(v, r);
                    }        
             }
             else{
                 y=0;
             }
//            pc.printf("ys %f, max_vel %f, yr %f, velocity %f, tar_rotations %f, position err %F\n\r", ys, max_vel, yr, velocity, rotation, position_err);
            startTime = t.read_us();
            prevPosition = position;
    }
}
