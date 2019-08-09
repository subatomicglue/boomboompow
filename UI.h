
class UI
{
   public:
      UI() {}
      bool button_tempoUp;
      bool button_tempoDown;
      bool button_track[8][4]; // 4 rows of 8 buttons ea
      bool button_playpause;
      bool button_playstop;
      bool button_pageflip;

      bool led_tempo;
      bool led_track[8][4]; // 4 rows of 8 buttons ea
      bool led_play;
      bool led_page[4];
      bool power;
};

#include "Wire.h"
#define PROCESS_INPUT( input, whichbit, whichvar ) \
   if ((input & (0x0001 << whichbit)) != 0) != whichvar) \
   { \
      whichvar = !whichvar; \
   }

class PiDriver
{
   public:
      UI* ui;

      void test()
      {
         // setup
         //Serial.begin( 9600 );
         Wire.begin(); // wake up I2C bus

         // set up IO pins
         Wire.beginTransmission( 0x20 );
         Wire.write( 0x00 ); // IODIRA register
         Wire.write( 0x00 ); // set all of bank A to outputs (8 bits here)
         Wire.endTransmission();

         Wire.beginTransmission( 0x20 );
         Wire.write( 0x01 ); // IODIRB register
         Wire.write( 0x00 ); // set all of bank B to outputs  (8 bits here)
         Wire.endTransmission();

         uint8_t x = 0;
         uint8_t inputs = 0x00;
         while (1)
         {
            Wire.beginTransmission( 0x20 );
            Wire.write( 0x12 ); // address bank A - GPIOA address on MCP23017
            Wire.write( x ); // value to send  (8 bits here)
            Wire.endTransmission();

            Wire.beginTransmission( 0x20 );
            Wire.write( 0x13 ); // address bank B - GPIOB address on MCP23017
            Wire.write( x ); // value to send (8 bits here)
            Wire.endTransmission();

            Wire.beginTransmission( 0x20 );
            Wire.write( 0x12 ); // address bank B - GPIOB address on MCP23017
            Wire.endTransmission();
            Wire.requestFrom( 0x20, 1 ); // request one byte of data
            uint8_t inputs07 = Wire.read();
            for (int x = 0; x < 8; ++x)
               PROCESS_INPUT( inputs07, x + 0, ui->button_track[x][0] );

            Wire.beginTransmission( 0x20 );
            Wire.write( 0x13 ); // address bank B - GPIOB address on MCP23017
            Wire.endTransmission();
            Wire.requestFrom( 0x20, 1 ); // request one byte of data
            uint8_t inputs815 = Wire.read();
            for (int x = 0; x < 8; ++x)
               PROCESS_INPUT( inputs815, x + 0, ui->button_track[x][1] );

            PROCESS_INPUT( 0, ui->button_tempoUp );
            PROCESS_INPUT( 1, ui->button_tempoDown );
            PROCESS_INPUT( 4, ui->button_playpause );
            PROCESS_INPUT( 4, ui->button_playstop );
            PROCESS_INPUT( 4, ui->button_pageflip );


            ++x;
         }
      }
};

class OglDriver
{
   public:
   UI* ui;
};

