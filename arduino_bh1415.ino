#include <ButtonGestures.h>

#define POWER_BUTTON 7
#define BUTTON_UP 8
#define BUTTON_DOWN 9
#define TX_ON_OFF 5

ButtonGestures  powerButton(POWER_BUTTON, LOW, INPUT_PULLUP);
ButtonGestures  upButton(BUTTON_UP, LOW, INPUT_PULLUP);
ButtonGestures  downButton(BUTTON_DOWN, LOW, INPUT_PULLUP);

#define data 12
#define clk 11
#define ena 10

unsigned int st_mn = 8; // Start with mono mode (8) and (9) for stereo
unsigned int frequency = 875; // Start with frequency 101.1 MHz
unsigned short st = 0; // Mono stereo control
unsigned int mask = 0; // Bit selection control for sending to BH1415

boolean is_on = false;
boolean is_menu = false;
boolean refresh_message = true;

void setup()
{
  Serial.begin(9600);

  pinMode(ena, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(TX_ON_OFF, OUTPUT);

  pinMode(POWER_BUTTON, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  digitalWrite(TX_ON_OFF, HIGH);  // Turn transmitter OFF by default
  while (!Serial);
  Serial.print("OFF |");
  Serial.println((st_mn == 8) ? " MONO" : " STEREO");
  Serial.println("Freq: " + String((float)frequency/10) + "MHz");
}

void loop()
{
  checkButtons();
  delay(100);
}

void statusPowerButton(const uint8_t state, const char* const label = NULL)  {
    switch (state) {
        case SINGLE_PRESS_SHORT: 
          if(is_menu && is_on) {
            Serial.print("Stereo/Mono: ");            
            Serial.println((st_mn == 8) ? "MONO" : "STEREO");
            Serial.println("Long press to change MONO/STEREO");
          } else if (!is_menu && is_on) {
            Serial.print(F("Double click to enter menu | Long press to turn if OFF"));
          } else {
            Serial.print(F("Long press to turn if ON"));
          }
        break;
        case SINGLE_PRESS_LONG:   
          if(is_on && !is_menu) {
            Serial.println(F(" turning OFF "));
            digitalWrite(TX_ON_OFF, HIGH);
            Serial.print("OFF |");
            Serial.println((st_mn == 8) ? " MONO" : " STEREO");
            Serial.println("Freq: " + String((float)frequency/10) + "MHz");
          } else if (is_menu) {
            st_mn = (st_mn == 8) ? 9 : 8;
            delay(200);
            setFrequency();
            delay(200);
            Serial.println((st_mn == 8) ? "MONO" : "STEREO");

          } else {
            digitalWrite(TX_ON_OFF, LOW);
            Serial.println(F(" turning ON "));
            delay(200);
            setFrequency();
            delay(200);            
            Serial.println("Freq: " + String((float)frequency/10) + "MHz");
          }
          is_on = !is_on;
        break;
        case DOUBLE_PRESS_SHORT:                  
          is_menu = !is_menu;
          is_menu ? Serial.println(F("Entering Menu")) : Serial.println(F("Leaving Menu")); 
        break;
        case NOT_PRESSED:
        default:
            return;
    }
    Serial.println();
}

void statusUpButton(const uint8_t state, const char* const label = NULL)  {
    switch (state) {
        case SINGLE_PRESS_SHORT: 
          if(is_on){
            if(is_menu) {

            } else {
              setFrequency();
              frequency++;
              if (frequency > 1080)
                frequency = 870;
              setFrequency();
              delay(200);
              setFrequency();
              delay(200);
              Serial.println("Freq: " + String((float)frequency/10) + "MHz");
            }
          }
        break;
        case SINGLE_PRESS_LONG: break;
        case DOUBLE_PRESS_SHORT: break;
        case NOT_PRESSED:
        default:
            return;
    }
    Serial.println();
}

void statusDownButton(const uint8_t state, const char* const label = NULL)  {
    switch (state) {
        case SINGLE_PRESS_SHORT: 
          if(is_on){
            if(is_menu) {
              Serial.print(F("Up in Menu"));            
            } else {
              frequency--;
              if (frequency < 870)
                frequency = 1080;
              setFrequency();
              delay(200);
              setFrequency();
              delay(200);            
              Serial.println("Freq: " + String((float)frequency/10) + "MHz");
            }
          }
        break;
        case SINGLE_PRESS_LONG: break;
        case DOUBLE_PRESS_SHORT:break;
        case NOT_PRESSED:
        default:
            return;
    }
    Serial.println();
}


void checkButtons()
{
  statusPowerButton(powerButton.check_button(), "Power Button");
  statusUpButton(upButton.check_button(), "Up Button");
  statusDownButton(downButton.check_button(), "Down Button");
}

void setFrequency() // Function to send bits to BH1415
{
  digitalWrite(ena, HIGH); // Chip Enable HIGH
  delayMicroseconds(10); // Initial delay after chip enable

  for (int n = 0; n < 11; n++) // Send frequency bits
  {
    mask = (1 << n); // MASK STARTS WITH 00000000000, EACH TIME THROUGH THE "FOR" LOOP, IT ADDS 1 BIT TO THE LEFT, 0000000001, 0000000010, 0000000100, ETC.
    // Select each bit to send
    if ((frequency & mask) != 0) // FREQUENCY IN BINARY 100.8 = 1111110000
    {
      // If the bit is 1, activate the DATA output, and the chip interprets it as 1
      digitalWrite(data, HIGH); // MASK = 0000000001
    }
    else
    {
      // If the bit is 0, deactivate the DATA output, and the chip interprets it as 0
      digitalWrite(data, LOW);
    }
    send();
  }
  for (int n = 0; n <= 4; n++) // Send controls
  {
    mask = (1 << n); // Select each bit to send
    //Serial.println(mask);
    if (st_mn & mask) // If the bit is 0
    {
      digitalWrite(data, HIGH); // Activate the DATA output
    }
    else
    {
      digitalWrite(data, LOW); // Deactivate the DATA output
    }
    send();
  }
  digitalWrite(ena, LOW); // Chip Enable LOW
}

void send() // Send each bit
{
  delayMicroseconds(10); // Wait for 10 microseconds
  digitalWrite(clk, HIGH); // Activate the clock output
  delayMicroseconds(10); // Wait for 10 microseconds
  digitalWrite(clk, LOW); // Deactivate the clock output
  delayMicroseconds(10); // Wait for 10 microseconds
}