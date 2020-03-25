#ifndef _motor_h_
#define _motor_h_

#include "mbed.h"
#include "bitcoin.h"

//Photointerrupter input pins
#define I1pin D3
#define I2pin D6
#define I3pin D5

//Incremental encoder input pins
#define CHApin   D12
#define CHBpin   D11

//Motor Drive output pins   //Mask in output byte
#define L1Lpin D1           //0x01
#define L1Hpin A3           //0x02
#define L2Lpin D0           //0x04
#define L2Hpin A6          //0x08
#define L3Lpin D10           //0x10
#define L3Hpin D2          //0x20

#define PWMpin D9

//Motor current sense
#define MCSPpin   A1
#define MCSNpin   A0

//Test outputs
#define TP0pin D4
#define TP1pin D13
#define TP2pin A2

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
//Mapping from sequential drive states to motor phase outputs
/*
State   L1  L2  L3
0       H   -   L
1       -   H   L
2       L   H   -
3       L   -   H
4       -   L   H
5       H   L   -
6       -   -   -
7       -   -   -
*/
//extern Serial pc;
extern float max_vel;
extern float rotation;
extern volatile bool rotationEnter;
extern PwmOut MotorPWM;

extern Thread motorCtrlT;
extern volatile int32_t position;
extern volatile float velocity;
extern void motorOut(int8_t driveState);
extern int8_t motorHome();
extern void GetSate_interrupt();
extern void ISR(void);
extern void setup();
extern void motorCtrlFn();
void motorCtrlTick();
float RotationControl();
float VelocityControl();


#endif