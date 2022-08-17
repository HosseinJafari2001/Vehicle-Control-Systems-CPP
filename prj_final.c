//for atmega32
#include <mega32.h>

// Alphanumeric LCD functions
#include <alcd.h>
#include <delay.h>
//for ftoa and ...
#include <stdio.h>
#include <stdlib.h>

//for calibre kardane damasanj
#define ADC_VREF_TYPE 0x00
#define calibr_lm35 4.836

char x=3,temp;
int t,f=10000;                              //bara makhzan
int timer_overflow=0,timer_overflow2=0;     //bara timer ha
//functions 
int key_pad(void); 
void Control_Motor(void);
void Direction_of_movement(void);
void Information(void);
void temperature(void);
void ultrosonic(void);
void fuel(void);
void ADC_Init();    
int ADC_Read(char channel);

// timer 0 , 1 interrupt
interrupt[TIM0_OVF] void timer1_ovf_isr(void)
{
#asm("cli")
if(timer_overflow>500)
{
    timer_overflow=0;
    ultrosonic();
    temperature();
    timer_overflow=0;
}
timer_overflow++;
TCNT0=0;
#asm("sei")
}
interrupt[TIM2_OVF] void timer2_ovf_isr(void)
{

    t++;
    timer_overflow2++;
    TCNT2=0;
}

// External Interrupt 0 service routine
interrupt [EXT_INT0] void ext_int0_isr(void)
{
    delay_ms(10);
    switch(x)
      {
      case 1 :
            if(OCR0<=(225)) {OCR0=OCR0+20;}
            break;
      case 2:
            if(OCR2<=(225)) {OCR2=OCR2+20;}
            break;
      case 3: 
            if((OCR0<=(225))&&(OCR2<=(225))) {OCR0=OCR0+20; OCR2=OCR2+20;}
            break; 
      }
            
}

// External Interrupt 1 service routine
interrupt [EXT_INT1] void ext_int1_isr(void)
{
    delay_ms(10);
    switch(x)
      {
      case 1 :
            if(OCR0>30) {OCR0=OCR0-20;}
            break;
      case 2:
            if(OCR2>30) {OCR2=OCR2-20;}
            break;
      case 3: 
            if((OCR0>30)&&(OCR2>30)) {OCR0=OCR0-20; OCR2=OCR2-20;}
            break;
      } 
}

void main(void)
{
int i=1;
DDRB=0xff;
DDRA=0xfa;
PORTA.2=0; 
DDRC=0xf0; PORTC=0xff;
DDRD=0xf3; PORTD.2=1; PORTD.3=1;    PORTD.0=1; PORTD.1=0;

// ADC initialization
// ADC Clock frequency: 1000.000 kHz
// ADC Voltage Reference: AREF pin
// ADC Auto Trigger Source: ADC Stopped
ADMUX=ADC_VREF_TYPE & 0xff;
ADCSRA=0x83;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 1000.000 kHz
// Mode: Fast PWM top=0xFF
// OC0 output: Non-Inverted PWM
// Timer Period: 0.256 ms
// Output Pulse(s):
// OC0 Period: 0.256 ms Width: 0 us
TCCR0=(1<<WGM00) | (1<<COM01) | (0<<COM00) | (1<<WGM01) | (0<<CS02) | (1<<CS01) | (0<<CS00);
TCNT0=0x00;
OCR0=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: 1000.000 kHz
// Mode: Fast PWM top=0xFF
// OC2 output: Non-Inverted PWM
// Timer Period: 0.256 ms
// Output Pulse(s):
// OC2 Period: 0.256 ms Width: 0 us
ASSR=0<<AS2;
TCCR2=(1<<PWM2) | (1<<COM21) | (0<<COM20) | (1<<CTC2) | (0<<CS22) | (1<<CS21) | (0<<CS20);
TCNT2=0x00;
OCR2=0x00;




// External Interrupt(s) initialization
// INT0: On
// INT0 Mode: Low level
// INT1: On
// INT1 Mode: Low level
// INT2: Off
GICR|=(1<<INT1) | (1<<INT0) | (0<<INT2);
MCUCR=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
MCUCSR=(0<<ISC2);
GIFR=(1<<INTF1) | (1<<INTF0) | (0<<INTF2);
TIMSK=0b01000001;
// Analog Comparator initialization
// Analog Comparator: Off
// The Analog Comparator's positive input is
// connected to the AIN0 pin
// The Analog Comparator's negative input is
// connected to the AIN1 pin
ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
SFIOR=(0<<ACME);


lcd_init(20);
#asm("sei")

ADC_Init();               

while (1)
      {
       lcd_clear();
       lcd_gotoxy(0,0);
       lcd_puts("Home : ");
       lcd_gotoxy(0,1); 
       lcd_puts("1-Control Motor");
       lcd_gotoxy(0,2);
       lcd_puts("2-Direction");
       lcd_gotoxy(0,3);
       lcd_puts("3-Information");
       i=key_pad();
       if(i==1) {Control_Motor();}
       else if(i==2) {Direction_of_movement();}
       else if (i==3) {Information();}                        
      }
}

/*______________________________functions____________________________________*/

int key_pad(void)
{
    int S; 
    //get number
    while(1)
    {    
        PORTC=0xdf; 
        if(!PINC.2){while(PINC.2==0);S= 3;break;}
        if(!PINC.1){while(PINC.1==0);S= 6;break;}
        if(!PINC.0){while(PINC.0==0);S= 9;break;}
        PORTC=0xbf; 
        if(!PINC.3){while(PINC.3==0);S= 0;break;}
        if(!PINC.2){while(PINC.2==0);S= 2;break;}
        if(!PINC.1){while(PINC.1==0);S= 5;break;}
        if(!PINC.0){while(PINC.0==0);S= 8;break;}
        PORTC=0x7f;
        if(!PINC.2){while(PINC.2==0);S= 1;break;}
        if(!PINC.1){while(PINC.1==0);S=4;break;}
        if(!PINC.0){while(PINC.0==0);S=7;break;}
    }
    return S; 
}

void temperature(void)
{
        char Temperature[6];
        float celsius;
        while(1)
        {

           //for temperature
           celsius = ADC_Read(0)*calibr_lm35;
           celsius /= 10;
           ftoa(celsius,1,Temperature);
           //temperature protection section
           if(celsius>70&&celsius<80) {lcd_gotoxy(16,0); lcd_puts("!!!!");PORTD.5=1;}
           else if(celsius>80) {lcd_clear(); lcd_puts("warning !!");delay_ms(100);OCR0=0;OCR2=0;}
           else {lcd_gotoxy(16,0); lcd_puts(Temperature); break;}
           PORTD.5=0;
        }
    TIMSK=0b01000001;    //fa'al kardane vaghfeie 0 , 2
    #asm("cli")           
}

void Control_Motor(void)
{
    char j;
    //lcd
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_puts("1-Left ");
    lcd_gotoxy(0,1);
    lcd_puts("2-Right ");
    lcd_gotoxy(0,2);
    lcd_puts("3-Both ");
    lcd_gotoxy(0,3);
    lcd_puts("4-STOP");
    //control section
    j=key_pad();
    if((j==1)) {x=j;}
    else if(j==2) {x=j;}
    else if(j==3) {x=j;}
    else if(j==4) {PORTD.0=0;PORTD.1=0;OCR0=0;OCR2=0;}
}

void Direction_of_movement(void)
{   //lcd
    char j;
    PORTD.4=0;
    PORTA.6=0;
    PORTA.7=0;
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_puts("1-go ahead");
    lcd_gotoxy(0,1);
    lcd_puts("2-Return");
    lcd_gotoxy(0,2);
    lcd_puts("3-go right");
    lcd_gotoxy(0,3);
    lcd_puts("4-go left");
    //rastgard chapgard aghabgard va jologard
    j=key_pad();
    switch(j)
    {
        case(1):
                PORTD.0=1; PORTD.1=0; OCR0=0x80; OCR2=0x80; temp=1; break;
        case(2):
                PORTD.0=0; PORTD.1=1; OCR0=0x80; OCR2=0x80; temp=0; PORTD.4=1; break;
        case(3):
                PORTD.0=1-temp; PORTD.1=temp; OCR0=0xff; OCR2=0x80; PORTA.6=1; break;
        case(4):
                PORTD.0=1-temp; PORTD.1=temp; OCR0=0x80; OCR2=0xff; PORTA.7=1; break;
    }           
}

void Information(void)
{   //lcd and information
    char p[10]; 
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_puts("LeftSpeed:");
    lcd_gotoxy(12,0);
    ftoa(OCR0,1,p);
    lcd_puts(p);
    lcd_gotoxy(0,1);
    lcd_puts("RightSpeed:");
    lcd_gotoxy(12,1);
    ftoa(OCR2,1,p);
    lcd_puts(p);
    fuel();
    lcd_gotoxy(0,2);
    lcd_puts("Fuel:");
    lcd_gotoxy(12,2);
    ftoa(f,1,p);
    lcd_puts(p);
    lcd_gotoxy(0,3);
    lcd_puts("1-Back to Menu");
    key_pad();          
}
//active adc
void ADC_Init()
{                                                   /* Make ADC port as input */
    ADCSRA = 0x87;          /* Enable ADC, with freq/128  */
    ADMUX = 0x40;           /* Vref: Avcc, ADC channel: 0 */
}
//read adc
int ADC_Read(char channel)                            
{
    ADMUX = 0x40 | (channel & 0x07);   /* set input channel to read */
    ADCSRA |= (1<<ADSC);               /* Start ADC conversion */
    while (!(ADCSRA & (1<<ADIF)));     /* Wait until end of conversion by polling ADC interrupt flag */
    ADCSRA |= (1<<ADIF);               /* Clear interrupt flag */
    delay_ms(1);
   
    return ADCW;                       /* Return ADC word */
}

void ultrosonic(void)
{ 
      float mm=0;   
      int timer;
      char cmm[10];
      #asm("sei")
      TIMSK=0b01000000;
      PORTA.1=1;
      delay_us(10); 
      PORTA.1=0; 
      while(PINA.2==0);
      timer_overflow2=0;TCNT2=0;
      while(PINA.2==1){if(timer_overflow2>5){mm=1;break;}}
      timer=(timer_overflow2*256+TCNT2);
      if(mm==0)
      {PORTD.0=0;PORTD.1=0;OCR0=0;OCR2=0;
      mm=timer*0.17;
      ftoa(mm,1,cmm);
      lcd_gotoxy(14,2);
      lcd_puts(cmm);}
}

void fuel(void)
{
    int avg;
    avg=(OCR0+OCR2)/2;
    f=f-(avg*0.0001*t);
    t=0;
}