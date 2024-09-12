#define DEBUG_DELAY   1000      // in ms
#define MESSURELOOPS  50000     // How many times to measure the speed of the functions. NOTE: for wokwi simulation set it to 0!

const gpio_num_t display_pins[] = { GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18 };
const int M0_display_pins[]     = { (int)display_pins[0], (int)display_pins[1], (int)display_pins[2] };

/*
For every LED, two pins are defined to control which is anode and which is cathode. All other pins must be set to INPUT mode
to be in opendrain state.

NOTE:
  The pinindexes are 1-based here, while the pinnumbers are 0-based! This is because I use the sign of the pinindex to define
  which pin is anode(+) or cathode(-).
*/
const int8_t LEDS[][2] =
{
  {+1, -2},  {-1, +2},  {+2, -3},  {-2, +3},  {+1, -3},  {-1, +3}
};
const String names[] = {"Green1", "Red", "Blue", "Yellow", "White", "Green2"};

const size_t NUM_LEDS = sizeof(LEDS) / sizeof(LEDS[0]);
const size_t NUM_PINS = sizeof(display_pins) / sizeof(display_pins[0]);

const uint64_t M1_ALLPINS_MASK = (1ULL << display_pins[0]) | (1ULL << display_pins[1]) | (1ULL << display_pins[2]);

gpio_config_t M1_GPIOconf = {
  .pin_bit_mask = M1_ALLPINS_MASK,
  .mode = GPIO_MODE_INPUT
};


void LitLed_M0(int i, bool debug=false) {
  int pin1 = M0_display_pins[ abs(LEDS[i][0])-1 ];
  int pin2 = M0_display_pins[ abs(LEDS[i][1])-1 ];

  pinMode(pin1, OUTPUT);
  digitalWrite(pin1, LEDS[i][0]>0 ? HIGH : LOW);
  pinMode(pin2, OUTPUT);
  digitalWrite(pin2, LEDS[i][1]>0 ? HIGH : LOW);

  if (debug) {
    vTaskDelay(DEBUG_DELAY / portTICK_PERIOD_MS);
    Serial.printf("Led #%d: %s\n", i, names[i].c_str());
  }
  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
}
void LitLed_M1(int i, bool debug=false) {
  gpio_num_t pin1 = display_pins[ abs(LEDS[i][0])-1 ];
  gpio_num_t pin2 = display_pins[ abs(LEDS[i][1])-1 ];

  M1_GPIOconf.pin_bit_mask = (1ULL << pin1) | (1ULL << pin2);
  M1_GPIOconf.mode = GPIO_MODE_OUTPUT;
  gpio_config(&M1_GPIOconf);

  gpio_set_level(pin1, LEDS[i][0]>0 ? HIGH : LOW);
  gpio_set_level(pin2, LEDS[i][1]>0 ? HIGH : LOW);

  if (debug) {
    vTaskDelay(DEBUG_DELAY / portTICK_PERIOD_MS);
    Serial.printf("Led #%d: %s\n", i, names[i].c_str());
  }

  M1_GPIOconf.mode = GPIO_MODE_INPUT;
  gpio_config(&M1_GPIOconf);
}
void LitLed_M2(int i, bool debug=false) {
  gpio_num_t pin1 = display_pins[ abs(LEDS[i][0])-1 ];
  gpio_num_t pin2 = display_pins[ abs(LEDS[i][1])-1 ];

  gpio_set_direction(pin1, GPIO_MODE_OUTPUT);
  gpio_set_level(pin1, LEDS[i][0]>0 ? HIGH : LOW);

  gpio_set_direction(pin2, GPIO_MODE_OUTPUT);
  gpio_set_level(pin2, LEDS[i][1]>0 ? HIGH : LOW);

  if (debug) {
    vTaskDelay(DEBUG_DELAY / portTICK_PERIOD_MS);
    Serial.printf("Led #%d: %s\n", i, names[i].c_str());
  }
  gpio_set_direction(pin1, GPIO_MODE_INPUT);
  gpio_set_direction(pin2, GPIO_MODE_INPUT);
}


void AllOff_M0() {
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(M0_display_pins[i], INPUT);
  }
}
void AllOff_M1() {
  M1_GPIOconf.pin_bit_mask = M1_ALLPINS_MASK;
  M1_GPIOconf.mode = GPIO_MODE_INPUT;
  gpio_config(&M1_GPIOconf);
}
void AllOff_M2() {
  for (int i = 0; i < NUM_PINS; i++) {
    gpio_set_direction(display_pins[i], GPIO_MODE_INPUT);
  }
}


//------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Initialize the GPIO pins to INPUT
  AllOff_M0();

  Serial.println("--------------------------------------------------------------");
  Serial.println("Circle through all Leds using slowest method... ");

  // In debugmode the LitLed functions are printing out the names of the leds
  for (int i = 0; i < NUM_LEDS; i++) {
    LitLed_M0(i, true);
  }

  if (MESSURELOOPS > 0) {
    Serial.println("--------------------------------------------------------------");
    Serial.println("Messure speed for AllOff functions...");

    uint32_t t = micros();
    for (int i = 0; i < MESSURELOOPS; i++) {
      AllOff_M0();
    }
    Serial.printf("AllOff_M0: %luµs\n", (micros() - t)/MESSURELOOPS);

    t = micros();
    for (int i = 0; i < MESSURELOOPS; i++) {
      AllOff_M1();
    }
    Serial.printf("AllOff_M1: %luµs\n", (micros() - t)/MESSURELOOPS);

    t = micros();
    for (int i = 0; i < MESSURELOOPS; i++) {
      AllOff_M2();
    }
    Serial.printf("AllOff_M2: %luµs\n", (micros() - t)/MESSURELOOPS);

    Serial.println("--------------------------------------------------------------");
    Serial.println("Messure speed for LitLed functions...");

    t = micros();
    for (int i = 0; i < MESSURELOOPS; i++) {
      LitLed_M0(i % NUM_LEDS);
    }
    Serial.printf("LitLed_M0: %luµs\n", (micros() - t)/MESSURELOOPS);

    t = micros();
    for (int i = 0; i < MESSURELOOPS; i++) {
      LitLed_M1(i % NUM_LEDS);
    }
    Serial.printf("LitLed_M1: %luµs\n", (micros() - t)/MESSURELOOPS);

    t = micros();
    for (int i = 0; i < MESSURELOOPS; i++) {
      LitLed_M2(i % NUM_LEDS);
    }
    Serial.printf("LitLed_M2: %luµs\n", (micros() - t)/MESSURELOOPS);
  }     // if (MESSURELOOPS > 0)

  Serial.println("--------------------------------------------------------------");

  Serial.println("Now the loop() will show three leds, toggle between the six. Also circling through all methods.");
}

//------------------------------------------------------------------------------
uint32_t  toggleTime  = millis() + 1000;
int       toggleState = 0, toggleMethod = 0;
void loop() {
  if (millis() > toggleTime) {
    toggleTime = millis() + 1000;

    if (toggleState == 1) {   // Then it will be set to zero again, after this if statement
      toggleMethod = (toggleMethod + 1) % 3;    // There are three methods
    }

    toggleState = 1 - toggleState;
  }

  // Toogle between the top and the bottom three leds
  for (int i = 0; i < NUM_LEDS/2; i++) {
    int n = i * 2 + toggleState;
    switch (toggleMethod) {
      case 0: LitLed_M0(n); break;
      case 1: LitLed_M1(n); break;
      case 2: LitLed_M2(n); break;
    }
  }
}