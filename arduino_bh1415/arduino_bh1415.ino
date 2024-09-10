#include <ButtonManager.h>
#include <Adafruit_SSD1306.h>

// start editing !!!
String station = "Radio B99";

#define SCREEN_WIDTH 128 // TFT display width, in pixels
#define SCREEN_HEIGHT 64 // TFT display height, in pixels
// Declaration for SSD1315 display connected using I2C
#define OLED_RESET    -1 // Reset pin not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define your buttons (name, pin, mode, initial status, previous status)
Button buttons[3] = {
    {"power_button", 4, INPUT_PULLUP, HIGH, HIGH},
    {"up_button", 5, INPUT_PULLUP, HIGH, HIGH},
    {"down_button", 6, INPUT_PULLUP, HIGH, HIGH}
};
int totalButtons = 3;

// Initialize the ButtonManager (3 buttons, 1000 ms for long press)
ButtonManager buttonManager(buttons, totalButtons, 500);

// BH1415 
int TX_ON_OFF = 9; // pin that turns voltage ON for IC
int ENABLE = 10;
int CLOCK = 11;
int DATA = 12;

unsigned int stereo_mono = 8; // Start with mono mode (8) and (9) for stereo
unsigned int frequency = 999; // Start with frequency 101.1 MHz
unsigned short st = 0; // Mono stereo control
unsigned int mask = 0; // Bit selection control for sending to BH1415

boolean is_on = false;
// stop editing !!!

void setup() {

  for (int i = 0; i < totalButtons; i++) {
    pinMode(buttons[i].pin, buttons[i].mode);
  }
  pinMode(ENABLE, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(TX_ON_OFF, OUTPUT);

  digitalWrite(TX_ON_OFF, HIGH); // Turn OFF TX

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Replace 0x3C with your display's I2C address if different
  display.setTextColor(SSD1306_WHITE); 
  displayScreen();  
}

void loop() {
  buttonManager.checkButtons(clickHandler);
}

void displayScreen() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(station);

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println(String((float)frequency/10) + " MHz");

  display.setTextSize(1);
  display.setCursor(0, 50);
  is_on ? display.print("ON") : display.print("OFF");  
  (stereo_mono == 8) ? display.println( " | MONO") : display.println( " | STEREO");
  delay(50);
  display.display();
}

void clickHandler(Button button, String click_type) {
  if(click_type == "long") {
      if(button.name == "power_button") {
        if(is_on) {
          is_on = false;
          digitalWrite(TX_ON_OFF, HIGH); // turn off TX          
        } else {
          is_on = true;
          digitalWrite(TX_ON_OFF, LOW); // turn on TX
          setFrequency("same");          
        }        
     }   

    if(button.name == "up_button" || button.name == "down_button") {
      stereo_mono = (stereo_mono == 8) ? 9 : 8; // change MONO or STEREO
    }
    displayScreen();

  } else if (click_type == "short") {
    if(button.name == "up_button") {
      setFrequency("up");
    } else if (button.name == "down_button") {
      setFrequency("down");
    }
    displayScreen();
  }     
}

void setFrequency(String direction) // Function to send bits to BH1415
{

  if(direction == "up") {
    frequency++;
    if(frequency > 1080) frequency = 870;
  } else if (direction == "down") {
    frequency--;
    if(frequency < 870) frequency = 1080;
  }

  delay(200);

  digitalWrite(ENABLE, HIGH); // Chip ENABLE HIGH
  delayMicroseconds(10); // Initial delay after chip ENABLE

  for (int n = 0; n < 11; n++) // Send frequency bits
  {
    mask = (1 << n); // MASK STARTS WITH 00000000000, EACH TIME THROUGH THE "FOR" LOOP, IT ADDS 1 BIT TO THE LEFT, 0000000001, 0000000010, 0000000100, ETC.
    // Select each bit to send
    if ((frequency & mask) != 0) // FREQUENCY IN BINARY 100.8 = 1111110000
    {
      // If the bit is 1, activate the DATA output, and the chip interprets it as 1
      digitalWrite(DATA, HIGH); // MASK = 0000000001
    }
    else
    {
      // If the bit is 0, deactivate the DATA output, and the chip interprets it as 0
      digitalWrite(DATA, LOW);
    }
    send();
  }
  for (int n = 0; n <= 4; n++) // Send controls
  {
    mask = (1 << n); // Select each bit to send
    //Serial.println(mask);
    if (stereo_mono & mask) // If the bit is 0
    {
      digitalWrite(DATA, HIGH); // Activate the DATA output
    }
    else
    {
      digitalWrite(DATA, LOW); // Deactivate the DATA output
    }
    send();
  }
  digitalWrite(ENABLE, LOW); // Chip ENABLEble LOW
  delay(200);
}

void send() // Send each bit
{
  delayMicroseconds(10); // Wait for 10 microseconds
  digitalWrite(CLOCK, HIGH); // Activate the clock output
  delayMicroseconds(10); // Wait for 10 microseconds
  digitalWrite(CLOCK, LOW); // Deactivate the clock output
  delayMicroseconds(10); // Wait for 10 microseconds
}