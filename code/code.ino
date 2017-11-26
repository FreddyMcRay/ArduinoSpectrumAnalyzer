#define LOG_OUT 0
#define LIN_OUT 1
#define FFT_N 256
#define clock 9
#define reset 8

#include <FFT.h>
#include <math.h>

unsigned char grenzen[11] = {0,3,5,7,9,11,13,16,24,32,69};
byte output[10] = {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000};
byte columnHeight[9] = {B00000000, B00000001, B00000011, B00000111, B00001111, B00011111, B00111111, B01111111, B11111111};

void setBalken(unsigned char column, unsigned char height){                   //calculation of the height of each column
    unsigned char h = (unsigned char)map(height, 0, 20, 0, 8);
    output[column] = columnHeight[h];
}

void show(int loops)
{
  for(int i = 0; i < loops; i++)
  {
    Serial.print(output[i]);
    Serial.print("  ");
    Serial.println("");
    PORTD = output[i];    
    digitalWrite(clock,HIGH);
     delayMicroseconds(5);
    digitalWrite(clock,LOW);
  }
}

void setup() {
  Serial.begin(115200);                                             //use the serial port
  TIMSK0 = 0;                                                       //turn off timer0 for lower jitter
  ADCSRA = 0xe5;                                                    //set the adc to free running mode
  ADMUX = 0b01000000;                                               
  DIDR0 = 0x01;                                                     //turn off the digital input for 
  analogReference(EXTERNAL);
  DDRD=B11111111;
  pinMode(clock,OUTPUT);
  pinMode(reset,OUTPUT);
 digitalWrite(reset,HIGH);
  delayMicroseconds(5);
 digitalWrite(reset,LOW);
}

void loop() {
  while(1) {                                                        //reduces jitter
    cli();                                                          //UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) {                            //save 256 samples
      while(!(ADCSRA & 0x10));                                      //wait for adc to be ready
      ADCSRA = 0xf5;                                                //restart adc
      byte m = ADCL;                                                //fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m;                                         //form into an int
      k -= 0x0200;                                                  //form into a signed int
      k <<= 6;                                                      //form into a 16b signed int
      fft_input[i] = k;                                             //put real data into even bins
    }

    fft_window();                                                   // window the data for better frequency response
    fft_reorder();                                                  // reorder the data before doing the fft
    fft_run();                                                      // process the data in the fft
    fft_mag_lin();                                                  // take the output of the fft
    sei();

    fft_lin_out[0] = 0;
    fft_lin_out[1] = 0;

    for(unsigned char i = 0; i < 10; i++){
      int maxW = 0;
        for(unsigned char x = grenzen[i]; x < grenzen[i+1]; x++){
 
           if((unsigned char)fft_lin_out[x] > maxW){
            maxW = fft_lin_out[x];
           }
        }

      setBalken(i, maxW);
      //Serial.print(maxW);
      //Serial.print(" ");
    }
    show(10);
    //Serial.println("");
  }

}
