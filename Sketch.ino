#include "thingProperties.h"
#include <FastLED.h>
 
//Config
//------------
 
// Pins
//Her bestemmer vi hvilke Arduino-pinner de forskjellige komponentene bruker
#define LED_PIN        3   //Pinnen som LED-stripen er koblet til
#define MODE_BUTTON    2   //Knappen som bytter mellom modusene
#define POWER_BUTTON   5   //Knappen som slår LED-stripen av og på
#define TRIG_PIN       9   //Trigger-pinnen på ultralydsensoren
#define ECHO_PIN       10  //Echo-pinnen på ultralydsensoren
#define POT_PIN        A0  //Analog pin som potentiometeret er koblet til
 
// LEDs
//Her bestemmer vi hvor mange LEDer vi har og hvor mange forskjellige moduser
#define NUM_LEDS       10  //Antall LEDer på LED-stripen
#define NUM_MODES      6   //Antall forskjellige lysmoduser
 
// Timing
//Disse verdiene bestemmer hvor ofte forskjellige ting skal oppdateres
const unsigned long BUTTON_DEBOUNCE    = 200;
const unsigned long ANIMATION_INTERVAL = 40;
const unsigned long DISTANCE_INTERVAL  = 100;
const unsigned long POT_INTERVAL       = 100;
 
//BUTTON_DEBOUNCE brukes for å unngå at ett knappetrykk registreres flere ganger
//ANIMATION_INTERVAL bestemmer hvor ofte LED-animasjonene oppdateres
//DISTANCE_INTERVAL bestemmer hvor ofte avstandssensoren leses
//POT_INTERVAL bestemmer hvor ofte potentiometeret leses
 
 
// Distance
//Her bestemmer vi minimum og maksimum avstand vi bruker fra ultralydsensoren
const int MIN_DISTANCE = 5;
const int MAX_DISTANCE = 100;
 
//Sensoren bruker altså avstander fra 5 cm til 100 cm
 
//Lager en liste som inneholder fargen til hver LED
CRGB leds[NUM_LEDS];
 

// START VALUES
//----------------
 
//currentMode bestemmer hvilken lysmodus som brukes
//Vi starter på modus 1
uint8_t currentMode = 1;
 
//Bestemmer om den fysiske strømknappen er på eller av
//true betyr at LED-stripen er på
bool physicalPower = true;
 
//Knappene bruker HIGH når de ikke er trykket og LOW når de er trykket
//Derfor starter begge knappene på HIGH
bool lastModeButtonState  = HIGH;
bool lastPowerButtonState = HIGH;
 
//Lagrer tidspunktet for når knappene sist ble trykket
unsigned long lastModeButtonPress  = 0;
unsigned long lastPowerButtonPress = 0;
 
//Lagrer tidspunktet for når forskjellige ting sist ble oppdatert
unsigned long lastAnimation    = 0;
unsigned long lastDistanceRead = 0;
unsigned long lastPotRead      = 0;
 
 

// ANIMATION VALUES/
//--------------------

//Starter regnbuefargen på 0
uint8_t rainbowHue = 0;
 
//Bestemmer hvilken LED som skal være aktiv i running-modus
//Vi starter på LED nummer 0
uint8_t runningPosition = 0;
 
 

// SENSOR VALUES
//---------------------
 
//Starter avstanden på minimumsavstanden
float distanceCM = MIN_DISTANCE;
 
//Starter potentiometeret på maksimal lysstyrke
uint8_t potBrightness = 255;
 
 

// SETUP
//--------------------
 
//setup() kjøres én gang når Arduinoen starter
void setup() {
 
  
  // LED STRIP
  //------------------------
 
  //Forteller FastLED hvilken type LED-strip vi bruker,
  //hvilken pin den er koblet til og hvilken rekkefølge fargene kommer i
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds,NUM_LEDS);
 
  //Starter med å slå av alle LEDene
  FastLED.clear();
 
  //Sender den nye informasjonen til LED-stripen
  FastLED.show();
 
 
  
  // BUTTONS
  //--------------------
 
  //Setter knappene som INPUT_PULLUP
  //Det betyr at Arduinoen bruker en intern motstand
  pinMode(MODE_BUTTON, INPUT_PULLUP);
  pinMode(POWER_BUTTON, INPUT_PULLUP);
 
 
  
  // ULTRASONIC SENSOR
  //-----------------------
 
  //Setter trigger som output fordi Arduinoen skal sende signal
  pinMode(TRIG_PIN, OUTPUT);
 
  //Setter echo som input fordi Arduinoen skal lese signalet
  pinMode(ECHO_PIN, INPUT);
 
  //Sørger for at trigger-signalet starter LOW
  digitalWrite(TRIG_PIN, LOW);
 
 
  
  // POTENTIOMETER
  //------------------------
 
  //Setter potentiometeret som input
  pinMode(POT_PIN, INPUT);
 
 
 
  // ARDUINO CLOUD
  //--------------------
 
  //Starter variablene som er laget i Arduino IoT Cloud
  initProperties();
 
  //Starter forbindelsen til Arduino Cloud
  ArduinoCloud.begin(
    ArduinoIoTPreferredConnection
  );
 
  //Bestemmer hvor mye informasjon som skal vises i Serial Monitor
  setDebugMessageLevel(2);
 
  //Skriver informasjon om Cloud-forbindelsen
  ArduinoCloud.printDebugInfo();
}
 
 

// MAIN LOOP
//----------------------
 
//loop() kjører hele tiden så lenge Arduinoen er på
void loop() {
 
  //Oppdaterer forbindelsen med Arduino Cloud
  ArduinoCloud.update();
 
  //Sjekker om knappene har blitt trykket
  checkButtons();
 
  //Oppdaterer sensorene dersom riktig modus er aktiv
  updateSensors();
 
  //Sjekker om det er på tide å oppdatere LED-animasjonen
  if (physicalPower && millis() - lastAnimation >= ANIMATION_INTERVAL) {
 
    //Lagrer tidspunktet for denne oppdateringen
    lastAnimation = millis();
 
    //Oppdaterer LEDene
    updateLEDs();
  }
}
 
 

// BUTTON CONTROL
//------------------------
 
//Denne funksjonen sjekker begge knappene
void checkButtons() {
 
  //Sjekker knappen som bytter modus
  checkModeButton();
 
  //Sjekker knappen som slår strømmen av og på
  checkPowerButton();
}
 
 

// MODE BUTTON
//--------------------
 
//Sjekker statusen til ModeButton
void checkModeButton() {
 
  //Leser den nåværende tilstanden til knappen
  bool state = digitalRead(MODE_BUTTON);
 
  //Sjekker om knappen har gått fra HIGH til LOW
  //HIGH -> LOW betyr at knappen har blitt trykket
  if (lastModeButtonState == HIGH && state == LOW && millis() - lastModeButtonPress > BUTTON_DEBOUNCE) {
 
    //Lagrer tidspunktet for knappetrykket
    lastModeButtonPress = millis();
 
    //Går til neste lysmodus
    nextMode();
  }
 
  //Lagrer knappens nåværende tilstand
  //Denne brukes neste gang funksjonen kjører
  lastModeButtonState = state;
}
 
 

// POWER BUTTON
//-------------------------
 
//Sjekker statusen til PowerButton
void checkPowerButton() {
 
  //Leser den nåværende tilstanden til knappen
  bool state = digitalRead(POWER_BUTTON);
 
  //Sjekker om knappen har gått fra HIGH til LOW
  //og om det har gått nok tid siden forrige trykk
  if (lastPowerButtonState == HIGH && state == LOW && millis() - lastPowerButtonPress > BUTTON_DEBOUNCE
  ) {
 
    //Lagrer tidspunktet for knappetrykket
    lastPowerButtonPress = millis();
 
    //Bytter mellom true og false
    //true = på
    //false = av
    physicalPower = !physicalPower;
 
    //Hvis strømmen blir slått på
    if (physicalPower) {
 
      //Starter animasjonene på nytt
      resetAnimations();
 
      //Oppdaterer LED-stripen
      updateLEDs();
    }
 
    //Hvis strømmen blir slått av
    else {
 
      //Slår av alle LEDene
      turnOffLEDs();
    }
  }
 
  //Lagrer knappens nåværende tilstand
  lastPowerButtonState = state;
}
 
 

// MODE CONTROL
//---------------------
 
//Denne funksjonen går til neste modus
void nextMode() {
 
  //Øker currentMode med 1
  currentMode++;
 
  //Hvis currentMode blir større enn antall moduser
  //starter vi på modus 1 igjen
  if (currentMode > NUM_MODES) {
    currentMode = 1;
  }
 
  //Starter animasjonene fra begynnelsen
  resetAnimations();
 
  //Slår av LEDene før den nye modusen starter
  turnOffLEDs();
}
 
 
//Starter animasjonene på nytt
void resetAnimations() {
 
  //Regnbuen starter på begynnelsen
  rainbowHue = 0;
 
  //Running-modus starter på LED 0
  runningPosition = 0;
}
 
 

// SENSOR CONTROL
//-------------------------
 
//Oppdaterer sensorene som brukes i de forskjellige modusene
void updateSensors() {
 
  //Hvis LED-stripen er avslått, trenger vi ikke lese sensorene
  if (!physicalPower) {
    return;
  }
 
  //Sjekker om vi er i modus 5 og om det er tid for en ny avstandsmåling
  if (
    currentMode == 5 &&
    millis() - lastDistanceRead >= DISTANCE_INTERVAL
  ) {
 
    //Lagrer tidspunktet for målingen
    lastDistanceRead = millis();
 
    //Leser avstanden fra ultralydsensoren
    readUltrasonic();
  }
 
  //Sjekker om vi er i modus 6 og om det er tid for å lese potentiometeret
  if (
    currentMode == 6 &&
    millis() - lastPotRead >= POT_INTERVAL
  ) {
 
    //Lagrer tidspunktet for avlesningen
    lastPotRead = millis();
 
    //Leser verdien fra potentiometeret
    readPotentiometer();
  }
}
 
 

// CLOUD HELPERS
//--------------------
 
//Henter fargen som er valgt i Arduino Cloud
CRGB getCloudColor() {
 
  //Lager tre variabler for rød, grønn og blå
  uint8_t r, g, b;
 
  //Henter RGB-verdiene fra fargen som er valgt i Cloud
  ledStrip.getValue().getRGB(r, g, b);
 
  //Returnerer fargen som en CRGB-verdi
  return CRGB(r, g, b);
}
 
 
//Henter lysstyrken som er valgt i Arduino Cloud
uint8_t getCloudBrightness() {
 
  //Henter brightness fra Cloud
  //constrain passer på at verdien er mellom 0 og 100
  int brightness = constrain(
    ledStrip.getBrightness(),
    0,
    100
  );
 
  //Arduino Cloud bruker 0-100
  //FastLED bruker 0-255
  //Derfor gjør vi om verdien her
  return map(
    brightness,
    0,
    100,
    0,
    255
  );
}
 
 

// ULTRASONIC
//--------------------------------------
 
//Leser avstanden fra ultralydsensoren
void readUltrasonic() {
 
  //Setter trigger LOW først for å starte en ny måling
  digitalWrite(TRIG_PIN, LOW);
 
  //Venter 2 mikrosekunder
  delayMicroseconds(2);
 
  //Sender en kort puls til sensoren
  digitalWrite(TRIG_PIN, HIGH);
 
  //Pulsen varer i 10 mikrosekunder
  delayMicroseconds(10);
 
  //Setter trigger tilbake til LOW
  digitalWrite(TRIG_PIN, LOW);
 
  //Måler hvor lenge ECHO-signalet er HIGH
  //30000 er en timeout slik at programmet ikke venter for alltid
  unsigned long duration = pulseIn(
    ECHO_PIN,
    HIGH,
    30000
  );
 
  //Hvis duration er 0 fikk sensoren ikke noe svar
  if (duration == 0) {
    return;
  }
 
  //Regner ut avstanden i centimeter
  //0.0343 er lydens hastighet i cm per mikrosekund
  //Vi deler på 2 fordi lyden går frem og tilbake
  distanceCM = (duration * 0.0343) / 2.0;
 
  //Begrenser avstanden til 5-100 cm
  distanceCM = constrain(
    distanceCM,
    MIN_DISTANCE,
    MAX_DISTANCE
  );
}
 
 

// POTENTIOMETER
//-----------------------
 
//Leser verdien fra potentiometeret
void readPotentiometer() {
 
  //Leser den analoge verdien
  //Arduinoen gir en verdi mellom 0 og 1023
  int value = analogRead(POT_PIN);
 
  //Gjør om verdien fra 0-1023 til 0-255
  //Dette passer med FastLED sin lysstyrke
  potBrightness = map(
    value,
    0,
    1023,
    0,
    255
  );
}
 
 

// LED CONTROL
//----------------
 
//Slår av alle LEDene
void turnOffLEDs() {
 
  //Setter alle LEDene til svart
  FastLED.clear();
 
  //Sender informasjonen til LED-stripen
  FastLED.show();
}
 
 
//Oppdaterer LED-stripen basert på hvilken modus som er valgt
void updateLEDs() {
 
  //Hvis den fysiske strømmen er av
  //eller Cloud-bryteren er av, slås LEDene av
  if (!physicalPower || !ledStrip.getSwitch()) {
    turnOffLEDs();
    return;
  }
 
  //Henter fargen fra Arduino Cloud
  CRGB color = getCloudColor();
 
  //Henter lysstyrken fra Arduino Cloud
  uint8_t brightness = getCloudBrightness();
 
  //Henter antall LEDer som skal være aktive
  //og passer på at tallet er mellom 0 og 10
  int numberOfLeds = constrain(
    activeLeds,
    0,
    NUM_LEDS
  );
 
  //Velger hvilken funksjon som skal brukes
  //basert på currentMode
  switch (currentMode) {
 
    //Modus 1: fast lys
    case 1:
      updateSolidMode(
        color,
        brightness,
        numberOfLeds
      );
      break;
 
    //Modus 2: lys som beveger seg
    case 2:
      updateRunningMode(
        color,
        brightness,
        numberOfLeds
      );
      break;
 
    //Modus 3: pulserende lys
    case 3:
      updatePulseMode(
        color,
        brightness,
        numberOfLeds
      );
      break;
 
    //Modus 4: regnbue
    case 4:
      updateRainbowMode(
        brightness,
        numberOfLeds
      );
      break;
 
    //Modus 5: avstandssensor
    case 5:
      updateDistanceMode(
        brightness,
        numberOfLeds
      );
      break;
 
    //Modus 6: potentiometer
    case 6:
      updatePotBrightnessMode(
        color,
        numberOfLeds
      );
      break;
  }
}
 
 

// MODE 1 - SOLID
//-----------------------
 
//Modus 1 viser et vanlig fast lys
void updateSolidMode(
  CRGB color,
  uint8_t brightness,
  int numberOfLeds
) {
 
  //Setter lysstyrken til verdien fra Cloud
  FastLED.setBrightness(brightness);
 
  //Slår av alle LEDene først
  //Dette gjør at gamle farger ikke blir igjen
  fill_solid(
    leds,
    NUM_LEDS,
    CRGB::Black
  );
 
  //Slår på det valgte antallet LEDer
  //Alle får samme farge
  fill_solid(
    leds,
    numberOfLeds,
    color
  );
 
  //Viser resultatet på LED-stripen
  FastLED.show();
}
 
 

// MODE 2 - RUNNING
//-------------------------
 
//Modus 2 lager et lys som beveger seg gjennom LED-stripen
void updateRunningMode(
  CRGB color,
  uint8_t brightness,
  int numberOfLeds
) {
 
  //Setter lysstyrken
  FastLED.setBrightness(brightness);
 
  //Slår av alle LEDene
  fill_solid(
    leds,
    NUM_LEDS,
    CRGB::Black
  );
 
  //Sjekker at det finnes minst én aktiv LED
  if (numberOfLeds > 0) {
 
    //Tenner LEDen på den nåværende posisjonen
    leds[runningPosition] = color;
 
    //Flytter posisjonen én plass videre
    runningPosition++;
 
    //Hvis vi har kommet til slutten
    //starter vi på LED 0 igjen
    if (runningPosition >= numberOfLeds) {
      runningPosition = 0;
    }
  }
 
  //Viser resultatet
  FastLED.show();
}
 
 

// MODE 3 - PULSE
//-----------------------
 
//Modus 3 lager en pulserende effekt
void updatePulseMode(
  CRGB color,
  uint8_t brightness,
  int numberOfLeds
) {
 
  //beatsin lager en verdi som går opp og ned
  //Her går verdien mellom 20 og 255
  uint8_t pulse = beatsin8(
    30,
    20,
    255
  );
 
  //Regner ut lysstyrken basert på både
  //Cloud brightness og pulse
  uint8_t finalBrightness =
    ((uint16_t)brightness * pulse) / 255;
 
  //Setter den beregnede lysstyrken
  FastLED.setBrightness(finalBrightness);
 
  //Slår av alle LEDene først
  fill_solid(
    leds,
    NUM_LEDS,
    CRGB::Black
  );
 
  //Tenner ønsket antall LEDer med valgt farge
  fill_solid(
    leds,
    numberOfLeds,
    color
  );
 
  //Viser resultatet
  FastLED.show();
}
 
 

// MODE 4 - RAINBOW
//----------------------
 
//Modus 4 lager en regnbueeffekt
void updateRainbowMode(
  uint8_t brightness,
  int numberOfLeds
) {
 
  //Setter lysstyrken
  FastLED.setBrightness(brightness);
 
  //Lager regnbuefarger på de aktive LEDene
  //rainbowHue bestemmer hvilken del av regnbuen vi starter på
  //20 bestemmer hvor stor forskjell det er mellom fargene
  fill_rainbow(
    leds,
    numberOfLeds,
    rainbowHue,
    20
  );
 
  //LEDene som ikke er aktive skal være svarte
  for (
    int i = numberOfLeds;
    i < NUM_LEDS;
    i++
  ) {
    leds[i] = CRGB::Black;
  }
 
  //Øker hue med 1 slik at regnbuen beveger seg
  rainbowHue++;
 
  //Viser regnbuen
  FastLED.show();
}
 
 

// MODE 5 - ULTRASONIC
//-----------------------------------
 
//Modus 5 bruker avstandssensoren til å styre lyset
void updateDistanceMode(
  uint8_t baseBrightness,
  int numberOfLeds
) {
 
  //Hvis ingen LEDer skal være aktive
  //slår vi av alle LEDene
  if (numberOfLeds <= 0) {
    turnOffLEDs();
    return;
  }
 
 
 
  // BRIGHTNESS BASED ON DISTANCE
  //----------------------------------
 
  //Nær sensor = høy lysstyrke
  //Langt fra sensor = lavere lysstyrke
  uint8_t distanceBrightness = map(
    distanceCM,
    MIN_DISTANCE,
    MAX_DISTANCE,
    255,
    20
  );
 
  //Passer på at lysstyrken holder seg mellom 20 og 255
  distanceBrightness = constrain(
    distanceBrightness,
    20,
    255
  );
 
 
  
  // COLOR BASED ON DISTANCE
  // -------------------------------------------
 
  //Nær sensor = rød
  //Langt unna = blå
  //
  //Hue 0 er rød
  //Hue 160 er blå-ish
  uint8_t hue = map(
    distanceCM,
    MIN_DISTANCE,
    MAX_DISTANCE,
    0,
    160
  );
 
 
  
  // FINAL BRIGHTNESS
  //--------------------------------------------------
 
  //Kombinerer lysstyrken fra Cloud
  //med lysstyrken som kommer fra avstanden
  uint8_t finalBrightness =
    ((uint16_t)baseBrightness * distanceBrightness) / 255;
 
  //Setter den endelige lysstyrken
  FastLED.setBrightness(finalBrightness);
 
 
  //Lager en farge basert på hue
  //Saturation 255 betyr full fargemetning
  //Value 255 betyr maksimal verdi
  CRGB color = CHSV(
    hue,
    255,
    255
  );
 
 
  //Slår av alle LEDene først
  fill_solid(
    leds,
    NUM_LEDS,
    CRGB::Black
  );
 
  //Tenner ønsket antall LEDer
  fill_solid(
    leds,
    numberOfLeds,
    color
  );
 
  //Viser resultatet på LED-stripen
  FastLED.show();
}
 
 

// MODE 6 - POTENTIOMETER
//---------------------------
 
//Modus 6 bruker potentiometeret til å kontrollere lysstyrken
void updatePotBrightnessMode(
  CRGB color,
  int numberOfLeds
) {
 
  //Hvis ingen LEDer skal være aktive
  //slår vi av alle LEDene
  if (numberOfLeds <= 0) {
    turnOffLEDs();
    return;
  }
 
  //Potentiometeret bestemmer lysstyrken direkte
  //Verdien potBrightness er mellom 0 og 255
  FastLED.setBrightness(potBrightness);
 
  //Slår av alle LEDene først
  fill_solid(
    leds,
    NUM_LEDS,
    CRGB::Black
  );
 
  //Tenner ønsket antall LEDer med valgt farge
  fill_solid(
    leds,
    numberOfLeds,
    color
  );
 
  //Viser resultatet
  FastLED.show();
}
 
 

// ARDUINO CLOUD CALLBACKS
//-----------------------------
 
//Denne funksjonen blir automatisk kalt
//når verdien til ledStrip endres i Arduino Cloud
void onLedStripChange() {
 
  //Oppdaterer LED-stripen med de nye verdiene
  updateLEDs();
}
 
 
//Denne funksjonen blir kalt når activeLeds endres
void onActiveLedsChange() {
 
  //Passer på at activeLeds ikke blir mindre enn 0
  //eller større enn antall LEDer
  activeLeds = constrain(
    activeLeds,
    0,
    NUM_LEDS
  );
 
  //Oppdaterer LED-stripen
  updateLEDs();
}
 
 
//Denne funksjonen blir kalt når cycleMode endres i Arduino Cloud
void onCycleModeChange() {
 
  //Sjekker om cycleMode er satt til true
  if (cycleMode) {
 
    //Går til neste lysmodus
    nextMode();
 
    //Setter cycleMode tilbake til false
    //slik at den kan brukes igjen senere
    cycleMode = false;
  }
}
