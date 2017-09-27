/* 
Arduino RTC with 16Mhz external crystal oscillator and 7-segment display.
*/ 

// for simplicity, define pins as segments
#define A (1<<PD0)
#define B (1<<PD1)
#define C (1<<PD2)
#define D (1<<PD3)
#define E (1<<PD4)
#define F (1<<PD5)
#define G (1<<PD6)
#define H (1<<PD7)

#define DISPLAY_REFRESH_RATE	200

#define PIN_DISP_1		8
#define PIN_DISP_2		9
#define PIN_DISP_3		10
#define PIN_DISP_4		11

#define PIN_HOUR_BUTTON		14
#define PIN_MIN_BUTTON		15
#define PIN_SEC_BUTTON		16

// these variables are changed by interrupt-routine, so they must be declared as volatile
static volatile struct Time
{
   int	seconds;
   int	minute;
   int	hour;
} time;

//Button debounce variables
int buttonStateHour; //this variable tracks the state of the button, high if not pressed, low if pressed
int buttonStateMin; //this variable tracks the state of the button, high if not pressed, low if pressed
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers


void setup() 
{

 
 pinMode(PIN_DISP_1, OUTPUT);
 pinMode(PIN_DISP_2, OUTPUT);
 pinMode(PIN_DISP_3, OUTPUT);
 pinMode(PIN_DISP_4, OUTPUT);
 
 pinMode(PIN_HOUR_BUTTON, INPUT_PULLUP);
 pinMode(PIN_MIN_BUTTON, INPUT_PULLUP);
 
 time.seconds = 0;
 time.minute = 0;
 time.hour = 0;
 
 InitTimer1();
 
 sei();
}


void loop() 
{
   flashNumber(time.hour*100 + time.minute);
   delay(1000/DISPLAY_REFRESH_RATE);
 
   
  //sample the state of the button - is it pressed or not?
  buttonStateHour = digitalRead(PIN_HOUR_BUTTON);
  buttonStateMin = digitalRead(PIN_MIN_BUTTON);
 
  //filter out any noise by setting a time buffer
  if ( (millis() - lastDebounceTime) > debounceDelay) 
  { 
    //if the HOUR button has been pressed
    if ( (buttonStateHour == LOW) ) 
    {
      delay(50);
      time.hour++;
      if(time.hour >= 24)
      {
		time.hour = 0;
      }
      
      lastDebounceTime = millis(); //set the current time
    } 
    
    //if the MIN button has been pressed
    if ( (buttonStateMin == LOW) ) 
    {
      delay(50);
      time.minute++;
      if(time.minute >= 60)
      {
		time.minute = 0;
      }
      
      lastDebounceTime = millis(); //set the current time
    } 
  }
  
}

//set timer1 interrupt at 1Hz
void InitTimer1()
{  
   TCCR1A = 0;// set entire TCCR1A register to 0
   TCCR1B = 0;// same for TCCR1B
   TCNT1  = 0;//initialize counter value to 0
   // set compare match register for 1hz increments
   OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
   // turn on CTC mode
   TCCR1B |= (1 << WGM12);
   // Set CS12 and CS10 bits for 1024 prescaler
   TCCR1B |= (1 << CS12) | (1 << CS10);  
   // enable timer compare interrupt
   TIMSK1 |= (1 << OCIE1A);
}



// given a number, set the appropriate segments
void setChar(char c)
{
  switch(c)
  {
       case 0: DDRD=A|B|C|D|E|F;       break;
       case 1: DDRD=B|C;               break;
       case 2: DDRD=A|B|G|E|D;         break;
       case 3: DDRD=A|B|G|C|D;         break;
       case 4: DDRD=F|G|B|C;           break;
       case 5: DDRD=A|F|G|C|D;         break;
       case 6: DDRD=A|F|G|E|C|D;       break;
       case 7: DDRD=A|B|C;             break;
       case 8: DDRD=A|B|C|D|E|F|G;     break;
       case 9: DDRD=A|F|G|B|C;         break;
       
       default: DDRD=0;                break;
  }
}

// refresh each segment, 1 - 4
// This function must be called with a certain refresh rate, to avoid flickering of the display
void flashNumber(int num)
{     
      char i;
      for (i=0;i<4;i++)
      {
		setChar(num%10); // divide by 10 to get the remainder (modulo)
		digitalWrite(11-i, HIGH); // enable the i-th segment. As "i" changes, each segment will be refreshed. The segments are defined on pins 8 - 11
		_delay_ms(10); // time to leave the segment illuminated
		digitalWrite(11-i, LOW);// pull down immediately
		num=num/10; // decrease the number by 10
      }		     
}

//timer1 interrupt 1Hz
ISR(TIMER1_COMPA_vect)
{
   time.seconds++;	
   if (time.seconds >= 60)
   {
      time.seconds = 0; // reset every minute
      time.minute++;
   }
   
   if (time.minute >= 60)
   {
      time.minute = 0; // reset every hour
      time.hour++;
   }
   
   if (time.hour >= 24)
   {
      time.hour = 0; // reset every day
   }
}