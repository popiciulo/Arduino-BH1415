#include <ButtonManager.h>

// start editing !!!
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
int TX_ON_OFF = 9;
int ENABLE = 10;
int CLOCK = 11;
int DATA = 12;

unsigned int stereo_mono = 8; // Start with mono mode (8) and (9) for stereo
unsigned int frequency = 875; // Start with frequency 101.1 MHz
unsigned short st = 0; // Mono stereo control
unsigned int mask = 0; // Bit selection control for sending to BH1415

boolean is_on = false;
boolean is_menu = false;
boolean refresh_message = true;

// stop editing !!!

void setup() {
  Serial.begin(9600); // for debugging

  for (int i = 0; i < totalButtons; i++) {
    pinMode(buttons[i].pin, buttons[i].mode);
  }
  pinMode(ENABLE, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(TX_ON_OFF, OUTPUT);

  digitalWrite(TX_ON_OFF, HIGH); // Turn OFF TX
  while (!Serial);
  Serial.print("OFF |");
  Serial.println((stereo_mono == 8) ? " MONO" : " STEREO");
  Serial.println("Freq: " + String((float)frequency/10) + "MHz");  
}

void loop() {
  buttonManager.checkButtons(clickHandler);
}

void displayScreen() {
  Serial.println("Freq: " + String((float)frequency/10) + "MHz");
  is_on ? Serial.print("ON |") : Serial.print("OFF | ");
  Serial.println((stereo_mono == 8) ? " MONO" : " STEREO");
}

void clickHandler(Button button, String click_type) {
  
  if(button.name == "power_button" && click_type == "long") {
    if(is_on) { 
      digitalWrite(TX_ON_OFF, HIGH); // turn off TX
      is_on = false;
    } else {
      is_on = true;
      digitalWrite(TX_ON_OFF, LOW); // turn on TX
      setFrequency("same");
    }
  }

  if(button.name == "up_button") {
    if(click_type == "short") {
      setFrequency("up");
    } else if(click_type == "long") { // TX if off and long click on UP
      stereo_mono = (stereo_mono == 8) ? 9 : 8; // change MONO or STEREO
    } 
  } 

  if(button.name == "down_button") {
    if(click_type == "short") {
      setFrequency("down");
    } else if(click_type == "long") { // TX if off and long click
      stereo_mono = (stereo_mono == 8) ? 9 : 8; // change MONO or STEREO
    } 
  } 

  displayScreen();   
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