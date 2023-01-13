/*
 * B1 - PIN9 - OC1A - AH
 * B2 - PIN10 - OC1B - BH
 * B3 - PIN11 - OC2A - CH
 * D3 - PIN3 - CL
 * D4 - PIN4 - BL
 * D5 - PIN5 - AL
 * A0 - Phase C
 * A1 - Phase B
 * A2 - Phase A
 * AIN0 - PIN6 - 比较器正输入
 */

#define PWM_max_value      255
#define PWM_min_value      35
#define PWM_value    100

int PWM_IN_MAX = 2000;
int PWM_IN_MIN = 1000;
int PWM_INPUT = 1300;
int beeping_PWM_VALUE = 100;
int sequence_step = 0;
bool full_stop = false;
bool MOTOR_SPINNING = false;
bool ESC_MODE_ON = false;
int motor_off_counter = 0;
int motor_speed = 35;

void setup() {

  DDRD  |= 0x38;           // 00111000 Configure pins 3, 4 and 5 as outputs
  PORTD  = 0x00;           //pins 0 to 7 set to LOW
  DDRB  |= 0x0E;           // 00001110 Configure pins 9, 10 and 11 as outputs
  PORTB  &= 0x31;          //B00110001 D9, D10 and D11 to LOW

  ACSR   = 0x10;           // Disable and clear (flag bit) analog comparator interrupt

  delay(200);
 
  ESC_MODE_ON = true;  

  beep_1KHZ(400);
  delay(200);

  TCCR2A =  0;            //OC2A - D11 normal port.    
  TCCR1A =  0;            // OC1A and OC1B normal port
  
  TCCR2A =  0;            //OC2A - D11 normal port.    
  TCCR1A =  0;            // OC1A and OC1B normal port

  delay(3000);
  
  beep_1KHZ(100);
  delay(300);
  beep_2KHZ(100);
  delay(300);
  beep_3KHZ(100);
  delay(300); 
  
}//End of setup loop

// The ISR vector of the Analog comparator
ISR (ANALOG_COMP_vect) {
  for(int i = 0; i < 10; i++) {           //We checl the comparator output 10 times to be sure
    if(sequence_step & 1)             //If step = odd (0001, 0011, 0101) 1, 3 or 5
    {
      if(!(ACSR & B00100000)) i -= 1; //!B00100000 -> B11011111 ACO = 0 (Analog Comparator Output = 0)
    }
    else                              //else if step is 0, 2 or 4
    {
      if((ACSR & B00100000))  i -= 1; //else if B00100000 -> B11011111 ACO = 1 (Analog Comparator Output = 1)
    }
  }
  set_next_step();                    //set the next step of the sequence
  sequence_step++;                    //increment step by 1, next part of the sequence of 6
  sequence_step %= 6;                 //If step > 5 (equal to 6) then step = 0 and start over
}

void set_next_step(){        
  switch(sequence_step){
    case 0:
      AH_BL();
      BEMF_C_RISING();
      break;
    case 1:
      AH_CL();
      BEMF_B_FALLING();
      break;
    case 2:
      BH_CL();
      BEMF_A_RISING();
      break;
    case 3:
      BH_AL();
      BEMF_C_FALLING();
      break;
    case 4:
      CH_AL();
      BEMF_B_RISING();
      break;
    case 5:
      CH_BL();
      BEMF_A_FALLING();
      break;
  }
}//end of BLDC_move

void SET_PWM(byte width_value){
  if(width_value < PWM_min_value)
    width_value  = PWM_min_value;
  if(width_value > PWM_max_value)
    width_value  = PWM_max_value;
  OCR1A  = width_value;                   // Set pin 9  PWM duty cycle
  OCR1B  = width_value;                   // Set pin 10 PWM duty cycle
  OCR2A  = width_value;                   // Set pin 11 PWM duty cycle
}
 
void loop() {

  MOTOR_SPINNING = true; 
  full_stop = false;
  motor_off_counter = 0;

  if(MOTOR_SPINNING)
  {
    SET_PWM(PWM_value);
    int i = 2000;
    // Motor start
    while(i > 500) {
      delayMicroseconds(i);
      set_next_step();
      sequence_step++;
      sequence_step %= 6;
      i = i - 10;
    }
    motor_speed = PWM_value;
    ACSR |= 0x08;                    // Enable analog comparator interrupt  
    while(MOTOR_SPINNING) 
    {
      PWM_INPUT = constrain(PWM_INPUT,PWM_IN_MIN,PWM_IN_MAX);
      motor_speed = map(PWM_INPUT,PWM_IN_MIN,PWM_IN_MAX,PWM_min_value,PWM_max_value);
      SET_PWM(motor_speed);
    }
  }//end of if MOTOR_SPINNING

  if(!MOTOR_SPINNING)
  { 
   
  }//end of if !MOTOR_SPINNING
}//end of void loop

void BEMF_A_RISING(){  
  ADCSRA = (0 << ADEN);     // Disable the ADC module
  ADCSRB = (1 << ACME);     // Enable MUX select for negative input of comparator
  ADMUX = 2;                // Select A2 as comparator negative input
  ACSR |= 0x03;             // Set interrupt on rising edge*/
}
void BEMF_A_FALLING(){
  ADCSRA = (0 << ADEN);     // Disable the ADC module
  ADCSRB = (1 << ACME);     // Enable MUX select for negative input of comparator
  ADMUX = 2;                // Select A2 as comparator negative input
  ACSR &= ~0x01;            // Set interrupt on falling edge*/
}
void BEMF_B_RISING(){
  ADCSRA = (0 << ADEN);     // Disable the ADC module
  ADCSRB = (1 << ACME);     // Enable MUX select for negative input of comparator
  ADMUX = 1;                // Select A1 as comparator negative input
  ACSR |= 0x03;             // Set interrupt on rising edge
}
void BEMF_B_FALLING(){
  ADCSRA = (0 << ADEN);     // Disable the ADC module
  ADCSRB = (1 << ACME);     // Enable MUX select for negative input of comparator
  ADMUX = 1;                // Select A1 as comparator negative input
  ACSR &= ~0x01;            // Set interrupt on falling edge*/
}
void BEMF_C_RISING(){
  ADCSRA = (0 << ADEN);     // Disable the ADC module
  ADCSRB = (1 << ACME);     // Enable MUX select for negative input of comparator
  ADMUX = 0;                // Select A0 as comparator negative input
  ACSR |= 0x03;             // Set interrupt on rising edge
}
void BEMF_C_FALLING(){
  ADCSRA = (0 << ADEN);     // Disable the ADC module
  ADCSRB = (1 << ACME);     // Enable MUX select for negative input of comparator
  ADMUX = 0;                // Select A0 as comparator negative input
  ACSR &= ~0x01;            // Set interrupt on falling edge*/
}

/*
 * B1 - PIN9 - OC1A - AH
 * B2 - PIN10 - OC1B - BH
 * B3 - PIN11 - OC2A - CH
 * D3 - PIN3 - CL
 * D4 - PIN4 - BL
 * D5 - PIN5 - AL
 * A0 - Phase C
 * A1 - Phase B
 * A2 - Phase A
 * AIN0 - PIN6 - 比较器正输入
 */

//D9 PWM and D4 HIGH.  
void AH_BL(){
  PORTD = B00010000;      //Set D4 to HIGH and the rest to LOW
  TCCR2A =  0;            //OC2A - D11 normal port. 
  TCCR1A =  0x81;         //OC1A - D9 compare match noninverting mode, downcounting ,PWM 8-bit
}
//D9 PWM and D3 HIGH
void AH_CL(){
  PORTD = B00001000;      //Set D3 to HIGH and the rest to LOW
  TCCR2A =  0;            //OC2A - D11 normal port. 
  TCCR1A =  0x81;         //OC1A - D9 compare match noninverting mode, downcounting ,PWM 8-bit  
}
//D10 PWM and D3 HIGH
void BH_CL(){
  PORTD = B00001000;      //Set D3 to HIGH and the rest to LOW
  TCCR2A =  0;            //OC2A - D11 normal port. 
  TCCR1A =  0x21;         //OC1B - D10 compare match noninverting mode, downcounting ,PWM 8-bit 
}
//D10 PWM and D5 HIGH
void BH_AL(){  
  PORTD = B00100000;      //Set D5 to HIGH and the rest to LOW
  TCCR2A =  0;            // OC2A - D11 normal port. 
  TCCR1A =  0x21;         //OC1B - D10 compare match noninverting mode, downcounting ,PWM 8-bit 
}
//D11 PWM and D5 HIGH
void CH_AL(){ 
  PORTD = B00100000;      //Set D5 to HIGH and the rest to LOW
  TCCR1A =  0;            // OC1A and OC1B normal port
  TCCR2A =  0x81;         // OC2A - D11 compare match noninverting mode, downcounting ,PWM 8-bit  
}
//D11 PWM and D4 HIGH
void CH_BL(){  
  PORTD = B00010000;      //Set D5 to HIGH and the rest to LOW
  TCCR1A =  0;            // OC1A and OC1B normal port
  TCCR2A =  0x81;         // OC2A - D11 compare match noninverting mode, downcounting ,PWM 8-bit
}

void beep_1KHZ (int milliseconds)
{
  int x = 0;
  PORTD = B00001000;      //Set D2 (CL) to HIGH and the rest to LOW
  while (x < milliseconds)
  { 
    PORTB = B00000010;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(450);
    
    PORTB = B00000100;      //Set D10 (BH) to HIGH (AH) to LOW 
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D10 (BH) to HIGH (AH) to LOW 
    delayMicroseconds(450);
    x = x + 1;
  }
   PORTD = B00000000;      //Set D2 (CL) to HIGH and the rest to LOW
   PORTB = B00000000;      //Set D10 (BH) to HIGH (AH) to LOW 
}

void beep_2KHZ (int milliseconds)
{
  int x = 0;
  PORTD = B00001000;      //Set D2 (CL) to HIGH and the rest to LOW
  while (x < milliseconds)
  { 
    PORTB = B00000010;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(200);
    
    PORTB = B00000100;      //Set D10 (BH) to HIGH (AH) to LOW 
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D10 (BH) to HIGH (AH) to LOW 
    delayMicroseconds(200);

    PORTB = B00000010;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(200);
    
    PORTB = B00000100;      //Set D10 (BH) to HIGH (AH) to LOW 
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D10 (BH) to HIGH (AH) to LOW 
    delayMicroseconds(200);
    x = x + 1;
  }
   PORTD = B00000000;      //Set D2 (CL) to HIGH and the rest to LOW
   PORTB = B00000000;      //Set D10 (BH) to HIGH (AH) to LOW 
}

void beep_3KHZ (int milliseconds)
{
  int x = 0;
  PORTD = B00001000;      //Set D2 (CL) to HIGH and the rest to LOW
  while (x < milliseconds)
  { 
    PORTB = B00000010;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(150);

    PORTB = B00000010;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(150);

    PORTB = B00000010;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(150);

    PORTB = B00000010;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(150);

    PORTB = B00000010;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(50);
    PORTB = B00000000;      //Set D90 (AH) to HIGH (BH) to LOW
    delayMicroseconds(150);
    x = x + 1;
  }
   PORTD = B00000000;      //Set D2 (CL) to HIGH and the rest to LOW
   PORTB = B00000000;      //Set D10 (BH) to HIGH (AH) to LOW 
}
