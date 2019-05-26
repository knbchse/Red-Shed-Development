#include <mbed.h>
//===[Stepper Class]===
class Stepper {
private:
    InterruptIn endStop;
    DigitalOut dirPin;      // stepper direction pin
    DigitalOut stepPin;     // stepper atep pin
    bool direction;     // direction (0-1)
    bool on;            // true-motor on, false-motor off
    unsigned int pulseCount;        // no of pulses
    unsigned int stepMode;
    float delay;          // delay between switching magnets (in us)
    Ticker t;           // ticker timer

d
public:
    Stepper(PinName p0, PinName p1, PinName p2, bool d, unsigned int p, unsigned int s):        // direction pin, step pin, direction, delay(us)
        dirPin(p0), stepPin(p1), endStop(p2)       // setup stepper pins
    {
        dirPin = d;         // set initial direction
        on = false;         // initailally off
        pulseCount = 0;     // reset pulse count
        stepMode = s;
        delay = p;          // set initial delay
        endStop.mode(PullUp);
        endStop.fall( this, &Stepper::stop);

    }
    void pulse(void);       // declare functions
    void run(unsigned int period);
    void stop(void);
    void toggleDirection(void);
    void toggleOn(void);
    void runPulse(unsigned int n);
    void ramp(unsigned int c);
    void distance(unsigned int d);
    void initialise(void);
    void Rotate(void);
    void setDirection(int d);

};

void Stepper::pulse(void) {
    stepPin = !stepPin;     // do half a pulse
    pulseCount++;           // count 1
}

void Stepper::run(unsigned int period) {
    if (period != 0) {
        delay = period;
    }
    endStop.mode(PullUp);
    t.attach_us(this, &Stepper::pulse, delay);      // attach the pulse to the ticker timer
    on = true;
}

void Stepper::stop(void) {
    stepPin = 0;        // turn all pins off
    pulseCount = 0;     // reset pulse count
    t.detach();         // turn of ticker
    on = false;
}

void Stepper::toggleDirection(void) {
    dirPin = !dirPin;       // toggle direction
}

void Stepper::toggleOn(void) {
    if (on == false)            // if off
        Stepper::run(0);    // turn on
    else
        Stepper::stop();        // else turn off
}

void Stepper::runPulse(unsigned int n) {
    pulseCount = 0;
    Stepper::run(0);
    while (pulseCount <= n*stepMode*50) {
        wait(.01);
    }
    Stepper::stop();
}

void Stepper::ramp(unsigned int c)  {
    }

void Stepper::distance(unsigned int d)  {
    pulseCount = 0;
    int h = 400;
    while (h > 60)  {
        Stepper::run(h);
        wait(.01);
        h--;
    }
    Stepper::run(0);

    while (pulseCount <= ((d-5)*800)) {
        wait(.1);
        }
    while (pulseCount <= (d*800))  {
        Stepper::run(h);
        wait(.00001);
        h++;
    }
    Stepper::stop();
}

void Stepper::initialise(void) {
    Stepper::setDirection(0);
    endStop.mode(PullUp);
    Stepper::run(100);
    while (endStop == 1) {
    }
    Stepper::toggleDirection();
    Stepper::stop();
}

void Stepper::setDirection(int d) {
    dirPin= d;
}

//===[End of Stepper Class]===
int main() {
  Stepper leadscrew_motor1(PB_12, PA_11, PB_5, 0, 200, 32); // 3
  Stepper leadscrew_motor2(PB_1, PB_2, PB_3, 0, 200, 32); // 3
  Initialise();
  leadscrew_motor1.initialise();
  leadscrew_motor2.initialise();

  // put your setup code here, to run once:

  while(1) {
    leadscrew_motor1.run(250);
    leadscrew_motor2.run(250);

    // put your main code here, to run repeatedly:
  }
}
