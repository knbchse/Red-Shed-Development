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

Ticker t;
char str1[16];
int i;
void Store(char column, char level);
void Retrieve(char column, char level);
void toggleDirection(void);
void stepperInitialise(void);
void motorun(int duration);

void toggleDirection(void)
{
    direction = ~direction;
}

void motorun(int duration)
{
    for (int i = 0; i < duration; i++)
    {
        stepper.write(1);
        wait_ms(1);
        stepper.write(0);
        wait_ms(1);
    }
}

void stepperInitialise(void)
{
    while(1)
    {
        stepper.write(1);
        wait_ms(1);
        stepper.write(0);
        wait_ms(1);
        if (endstop == 0)
        {
            return;
        }
    }
}

//--------BEGINING OF LCD CLASS-----------//
class LCD
{
    private:
        DigitalOut lcdD4, lcdD5, lcdD6, lcdD7;
        DigitalOut lcdEN, lcdRS;
        Ticker lcdt;

    public:
        LCD(PinName p0, PinName p1, PinName p2, PinName p3, PinName p4, PinName p5):
            lcdD4(p0), lcdD5(p1), lcdD6(p2), lcdD7(p3), lcdEN(p4), lcdRS(p5) {}     // setup  pins
        void lcdCommand(unsigned char command);
        void lcdPutChar(unsigned char c);
        void Initialise(void);
        void lcdSetRS(int mode); //-- mode is either LCD_DATA or LCD_INSTRUCTION
        void lcdPulseEN(void);
        void lcdInit8Bit(unsigned char command);

};


void LCD::Initialise(void)
{
    lcdEN.write(0); //-- GPIO_WriteBit(GPIOC, LCD_EN, Bit_RESET);
    wait_us(15000); //-- delay for >15msec second after power on
    lcdInit8Bit(0x30); //-- we are in "8bit" mode
    wait_us(4100); //-- 4.1msec delay
    lcdInit8Bit(0x30); //-- but the bottom 4 bits are ignored
    wait_us(100); //-- 100usec delay
    lcdInit8Bit(0x30);
    lcdInit8Bit(0x20);
    lcdCommand(0x28); //-- we are now in 4bit mode, dual line
    lcdCommand(0x08); //-- display off
    lcdCommand(0x01); //-- display clear
    wait_us(2000); //-- needs a 2msec delay !!
    lcdCommand(0x06); //-- cursor increments
    lcdCommand(0x0c); //-- display on, cursor(blinks) off
}


void LCD::lcdSetRS(int mode)
{
    lcdRS.write(mode);
}


void LCD::lcdPulseEN(void)
{
    lcdEN.write(1);
    wait_us(1); //-- enable pulse must be >450ns
    lcdEN.write(0);
    wait_us(1);
}


void LCD::lcdInit8Bit(unsigned char command)
{
    lcdSetRS(LCD_INSTRUCTION);
    lcdD4.write((command>>4) & 0x01); //-- bottom 4 bits
    lcdD5.write((command>>5) & 0x01); //-- are ignored
    lcdD6.write((command>>6) & 0x01);
    lcdD7.write((command>>7) & 0x01);
    lcdPulseEN();
    wait_us(37); //-- let it work on the data
}


void LCD::lcdCommand(unsigned char command)
{
    lcdSetRS(LCD_INSTRUCTION);
    lcdD4.write((command>>4) & 0x01);
    lcdD5.write((command>>5) & 0x01);
    lcdD6.write((command>>6) & 0x01);
    lcdD7.write((command>>7) & 0x01);
    lcdPulseEN(); //-- this can't be too slow or it will time out
    lcdD4.write(command & 0x01);
    lcdD5.write((command>>1) & 0x01);
    lcdD6.write((command>>2) & 0x01);
    lcdD7.write((command>>3) & 0x01);
    lcdPulseEN();
    wait_us(37); //-- let it work on the data
}


void LCD::lcdPutChar(unsigned char c)
{
    lcdSetRS(LCD_DATA);
    lcdD4.write((c>>4) & 0x01);
    lcdD5.write((c>>5) & 0x01);
    lcdD6.write((c>>6) & 0x01);
    lcdD7.write((c>>7) & 0x01);
    lcdPulseEN(); //-- this can't be too slow or it will time out
    lcdD4.write(c & 0x01);
    lcdD5.write((c>>1) & 0x01);
    lcdD6.write((c>>2) & 0x01);
    lcdD7.write((c>>3) & 0x01);
    lcdPulseEN();
    wait_us(37); //-- let it work on the data
}


//-----------END OF LCD CLASS-------------//


int main()
{
    LCD lcd(PA_9, PA_8, PB_10, PB_4, PC_7, PB_6);
    stepperInitialise();
    direction = 0;
    endstop.mode(PullUp);
    lcd.Initialise();
    servo1.period_us(20000); //-- 20 ms time period
    servo1.pulsewidth_us(1000); //-- pulse width of 1 ms; 0 degrees'
    servo2.period_us(20000); //-- 20 ms time period
    servo2.pulsewidth_us(1000); //-- pulse width of 1 ms; 0 degrees'
    servo3.period_us(20000); //-- 20 ms time period
    servo3.pulsewidth_us(1000); //-- pulse width of 1 ms; 0 degrees'
    while (1) {

        char s[80];                   // Creates a buffer for uart input
        if (uart.canReadLine()) {     // I uart sends a string
            uart.readLine(s);         // Reads the input string
            s[strlen(s)-1] = '\0';    // Ends the buffer with a NULL character
        }

        if (s[0] == 'R')       {                            // If the first letter is 's'
            lcd.Initialise();                               // Init the LCD
            sprintf(str1, "Retrieving %c%c", s[1], s[2]);   // Outputs command to LCD
            i = 0;
            while(str1[i]) {                                // While the string is not finished
                lcd.lcdPutChar(str1[i]);                    // Outputs character to LCD
                i++;
            }
            Retrieve(s[1], s[2]);                           // Command to retrieve a pallete
            wait(2);
            lcd.lcdCommand(0x01); //-- display clear
            sprintf(str1, "D\n");
            uart.putString(str1);
            sprintf(str1, "Ready");
            i = 0;
            while(str1[i]) {
                lcd.lcdPutChar(str1[i]);
                i++;
            }
        }

        if (s[0] == 'S')  {
            lcd.Initialise();
            sprintf(str1, "Storing in %c%c", s[1], s[2]);
            i = 0;
            while(str1[i]) {
                lcd.lcdPutChar(str1[i]);
                i++;
            }
            Store(s[1], s[2]);
            wait(2);
            lcd.lcdCommand(0x01); //-- display clear
            sprintf(str1, "D\n");
            uart.putString(str1);
            sprintf(str1, "Ready");
            i = 0;
            while(str1[i]) {
                lcd.lcdPutChar(str1[i]);
                i++;
            }
        }
        s[0] = '0';
    }
}

void Store(char column, char level)
{
    if(level == 'A')
    {
        motorun(250); // Initial Height level A set
    }
    else
    {
        motorun(500); // Initial Height level B set
    }
    wait(1);
    toggleDirection();
    switch (column)
    {
        case '1' :  servo1.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorun(100); // Move shelf down slightly
                    wait(1);
                    servo1.pulsewidth_us(1000);// Servo in
                    break;
        case '2' :  servo2.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorun(100); // Move shelf down slightly
                    wait(1);
                    servo2.pulsewidth_us(1000);// Servo in
                    break;
        case '3' :  servo3.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorun(100); // Move shelf down slightly
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
        motorun(245); // Initial Height level A set
    }
    else
    {
        motorun(495); // Initial Height level B set
    }
    wait(1);
    switch (column)
    {
        case '1' :  servo1.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorun(100); // Move shelf down slightly
                    wait(1);
                    servo1.pulsewidth_us(1000);// Servo in
                    break;
        case '2' :  servo2.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorun(100); // Move shelf down slightly
                    wait(1);
                    servo2.pulsewidth_us(1000);// Servo in
                    break;
        case '3' :  servo3.pulsewidth_us(2000);// Servo out
                    wait(1);
                    motorun(100); // Move shelf down slightly
                    wait(1);
                    servo3.pulsewidth_us(1000);// Servo in
                    break;
        default  :  break;
    }
    toggleDirection();
    wait(1);
    stepperInitialise(); // Return Shelf to home
}
