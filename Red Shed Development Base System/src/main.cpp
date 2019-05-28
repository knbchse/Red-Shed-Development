#include "mbed.h"
#include "uart.h"
#include "mbed.h"
#include "string.h"
#define LCD_DATA 1
#define LCD_INSTRUCTION 0

PwmOut servo1(PA_0);
PwmOut servo2(PA_1);
PwmOut servo3(PB_0);
DigitalOut stepper(PA_11);
DigitalOut direction(PB_12);
DigitalIn  endstop(PB_5);
Uart uart(PA_2, PA_3);
int speed = 5000;
int constSpeed = speed;
Ticker t;
char str1[16];
int i;
void Store(char column, char level);
void Retrieve(char column, char level);
void stepperInitialise(void);
void motorunup(int duration);
void motorundown(int duration);

void motorunup(int duration)
{
    direction = 0;

    for (int i = 0; i < duration/4; i++)
    {
        stepper.write(1);
        wait_us(speed);
        stepper.write(0);
        wait_us(speed);
        speed = speed - 10;
    }
    for (int i = 0; i < duration*3/4; i++)
    {
        stepper.write(1);
        wait_us(speed);
        stepper.write(0);
        wait_us(speed);
    }
    for (int i = 0; i < duration; i++)
    {
        stepper.write(1);
        wait_us(speed);
        stepper.write(0);
        wait_us(speed);
        speed = speed + 10;
    }
    speed = constSpeed;
}

void motorundown(int duration)
{
    direction = 1;

    for (int i = 0; i < duration/4; i++)
    {
        stepper.write(1);
        wait_us(speed);
        stepper.write(0);
        wait_us(speed);
        speed = speed - 10;
    }
    for (int i = 0; i < duration*3/4; i++)
    {
        stepper.write(1);
        wait_us(speed);
        stepper.write(0);
        wait_us(speed);
    }
    for (int i = 0; i < duration; i++)
    {
        stepper.write(1);
        wait_us(speed);
        stepper.write(0);
        wait_us(speed);
        speed = speed + 10;
    }
    speed = constSpeed;
}

void stepperInitialise(void)
{
    direction = 1;
    for (int i = 0; i < 1000; i++)
    {
        stepper.write(1);
        wait_us(speed);
        stepper.write(0);
        wait_us(speed);
        speed = speed - 10;
    }
    while(1)
    {
        stepper.write(1);
        wait_us(speed);
        stepper.write(0);
        wait_us(speed);
        if (endstop == 0)
        {
            return;
        }
    }
}

int main()
{
    direction = 1;
    endstop.mode(PullUp);
    servo1.period_us(20000); //-- 20 ms time period
    servo1.pulsewidth_us(1000); //-- pulse width of 1 ms; 0 degrees'
    servo2.period_us(20000); //-- 20 ms time period
    servo2.pulsewidth_us(1000); //-- pulse width of 1 ms; 0 degrees'
    servo3.period_us(20000); //-- 20 ms time period
    servo3.pulsewidth_us(1000); //-- pulse width of 1 ms; 0 degrees'
    stepperInitialise();
    while (1) {

        char s[80];                   // Creates a buffer for uart input
        if (uart.canReadLine()) {     // I uart sends a string
            uart.readLine(s);         // Reads the input string
            s[strlen(s)-1] = '\0';    // Ends the buffer with a NULL character
        }

        if (s[0] == 'R')       {                            // If the first letter is 's'
            Retrieve(s[1], s[2]);                           // Command to retrieve a pallete
            wait(2);
            sprintf(str1, "D\n");
            uart.putString(str1);
        }

        if (s[0] == 'S')  {
            Store(s[1], s[2]);
            wait(2);
            sprintf(str1, "D\n");
            uart.putString(str1);
        }
        s[0] = '0';
    }
}

void Store(char column, char level)
{
    if(level == 'A')
    {
        motorunup(7000); // Initial Height level A set
    }
    else
    {
        motorunup(13000); // Initial Height level B set
    }
    wait(1);
    switch (column)
    {
        case '1' :  servo1.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorundown(1000); // Move shelf down slightly
                    wait(1);
                    servo1.pulsewidth_us(1000);// Servo in
                    break;
        case '2' :  servo2.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorundown(1000); // Move shelf down slightly
                    wait(1);
                    servo2.pulsewidth_us(1000);// Servo in
                    break;
        case '3' :  servo3.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorundown(1000); // Move shelf down slightly
                    wait(1);
                    servo3.pulsewidth_us(1000);// Servo in
                    break;
        default  :  break;

    }
    wait(1);
    stepperInitialise(); // Return Shelf to home
}

void Retrieve(char column, char level)
{
    if(level == 'A')
    {
        motorunup(3000); // Initial Height level A set
    }
    else
    {
        motorunup(6000); // Initial Height level B set
    }
    wait(1);
    switch (column)
    {
        case '1' :  servo1.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorunup(1000); // Move shelf down slightly
                    wait(1);
                    servo1.pulsewidth_us(1000);// Servo in
                    break;
        case '2' :  servo2.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorunup(1000); // Move shelf down slightly
                    wait(1);
                    servo2.pulsewidth_us(1000);// Servo in
                    break;
        case '3' :  servo3.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorunup(1000); // Move shelf down slightly
                    wait(1);
                    servo3.pulsewidth_us(1000);// Servo in
                    break;
        default  :  break;
    }
    wait(1);
    stepperInitialise(); // Return Shelf to home
}
