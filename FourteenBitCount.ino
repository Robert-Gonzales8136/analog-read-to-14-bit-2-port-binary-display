/*
This code maps an analog 10 bit value from A0 to a 14 binary bit value displayed on 14 LEDs on port D0-D8 and port B0-B6.
The defined function is a macro to make masks to access specific bits on a port. OR with a mask will set bit X and NAND with a mask will clear bit X.
Bit masking is to guarantee that unused bits are not changed.
The unions are bit fields that are a full word long. The 2 most significant bits are unused on port B so they're unknown, unnamed, unchanged and PB is specified as a partial port.
When the port values are loaded into a word, 6 bits of port B are shifted one byte and added to the full port D.
Bit masking and bit field techniques taken from the LinkedIn Learning tutorial "C Programming for Embedded Applications" by Eduardo Corpeño.
 */
#include <stdlib.h>

//Macro program that creates a mask to access bit x.
#define MASK(x) ((unsigned char) (1<<(x))) 

//Bit field to store only needed port bits.
typedef union {
  uint16_t Word;
  struct {
    uint16_t PD        :8;
    uint16_t           :8;
  } Port;
  struct {
    uint16_t           :8;
    uint16_t PB        :6;
    uint16_t           :2;
  } PartialPort;
} MultiPortWord;

//Bit field to store previous state of ports.
typedef union {
  uint16_t Word;
  struct {
    uint16_t PD        :8;
    uint16_t           :8;
  } Port;
  struct {
    uint16_t           :8;
    uint16_t PB        :6;
    uint16_t           :2;
  } PartialPort;
} LastMultiPortWord;

MultiPortWord reg;
LastMultiPortWord lastReg;

void setup(){
  //DDRB stands for Data Direction Register port B. OR with a bit mask here will set a pin as input.
  
  //Port B:
  DDRB |= MASK(5);   // The LED pin is output. Mask: 00100000
  DDRB |= MASK(4);   // The LED pin is output. Mask: 00010000                
  DDRB |= MASK(3);   // The LED pin is output. Mask: 00001000
  DDRB |= MASK(2);   // The LED pin is output. Mask: 00000100                
  DDRB |= MASK(1);   // The LED pin is output. Mask: 00000010
  DDRB |= MASK(0);   // The LED pin is output. Mask: 00000001
  //Port D:                
  DDRD |= MASK(7);   // The LED pin is output. Mask: 10000000
  DDRD |= MASK(6);   // The LED pin is output. Mask: 01000000                
  DDRD |= MASK(5);   // The LED pin is output. Mask: 00100000
  DDRD |= MASK(4);   // The LED pin is output. Mask: 00010000                
  DDRD |= MASK(3);   // The LED pin is output. Mask: 00001000
  DDRD |= MASK(2);   // The LED pin is output. Mask: 00000100                
  DDRD |= MASK(1);   // The LED pin is output. Mask: 00000010
  DDRD |= MASK(0);   // The LED pin is output. Mask: 00000001

  /*The previous word is initialized as the current contents of the ports.
  This is in the unlikely case that either port is not clear at startup, the bits will clear after recieving the first measured value.*/
  lastReg.Word = (PORTB << 8) + PORTD;
}
  
void loop(){
  uint16_t sensorValue = analogRead(A0);
 
  //The current word is mapped to be a 14 bit value from the 10 bit analog value.
  reg.Word = map(sensorValue, 0, 1023, 0, 16383);
  
  /*When the ports change, the HIGH bits from the previous word clear the corresponding values on the ports
  and the current word sets the cleared ports.
  Previous word is updated with the value of the current word without including the flags in the MSBs of port B*/
  if(reg.Word != lastReg.Word){
    PORTD &= ~lastReg.Port.PD; //change to 255 to clear all port D
    PORTB &= ~lastReg.PartialPort.PB; //change to 63 to clear all port B
    PORTD |= reg.Port.PD;
    PORTB |= reg.PartialPort.PB;
    
    /*Since the current word is set by mapping, the value of the unused bits in port B could yield unwanted results like infinitely looping the if statement or changing those flag
    values unexpectedy if we didn't combine the partial port with the full port.*/
    lastReg.Word = (reg.PartialPort.PB << 8) + reg.Port.PD;
  }
}
