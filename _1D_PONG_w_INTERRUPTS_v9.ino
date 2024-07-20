// 1D PONG - MICHAL SEJC

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978


#define MAX_SCORE 3
#define INCREMENT_DELAY_HIT_TRESHOLD 1

#define LEFT 0
#define RIGHT 1
#define ANYONE 2
#define NO_ONE 3
#define BOOTUP 4
#define WIN 5
#define MISS 6


#define VIOLATION_BLINK_PERIOD 45
#define WAITING_BLINK_PERIOD 200

#define BLINKS_PER_VIOLATION 10

#define DELAY_INITIAL 150
#define DELAY_MINIMAL 70
#define DELAY_SUBTRAHEND 15


#define LEFT_BTN_PIN 2
#define RIGHT_BTN_PIN 3
#define BUZ_PIN 4

#define N_OF_LEDS 8
byte LED[N_OF_LEDS] = {6, 7, 8, 9, 10, 11, 12, 5};



byte scoreBuffer = 0b00000000;
byte positionBuffer;
byte LEDbuffer;
byte temp;
byte i;

byte startPattern[2] = {0b10000000, 0b00000001};
byte score[2];
byte violator;

int hitCount;
int gameDelay = DELAY_INITIAL;

byte startingSide;


void setup() {
    Serial.begin(9600); 
  pinMode(LEFT_BTN_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZ_PIN, OUTPUT);

  for (byte i = 0; i < N_OF_LEDS; i++)
    pinMode(LED[i], OUTPUT);

  attachInterrupt(0, leftBtnInterrupt, FALLING);
  attachInterrupt(1, rightBtnInterrupt, FALLING);
}

volatile boolean btnState[2] = {0,0};

void leftBtnInterrupt () {
  btnState[LEFT] = 1;
}
void rightBtnInterrupt () {
  btnState[RIGHT] = 1;
}

void updateLEDpins() {
  for(byte i = 0; i < N_OF_LEDS; i++)
    digitalWrite(LED[i], bitRead(LEDbuffer, N_OF_LEDS - 1 - i));
}

void blinkLEDsAwaiting(int who) {
  LEDbuffer = scoreBuffer;
  updateLEDpins();
  delay(WAITING_BLINK_PERIOD);
  positionBuffer = startPattern[who];
  LEDbuffer |= positionBuffer;
  updateLEDpins();
  delay(WAITING_BLINK_PERIOD);
}

void signalViolation() {
  for(i = 0; i < BLINKS_PER_VIOLATION; i++)
  {
    LEDbuffer = 0b00000000;
    updateLEDpins();
    tone(BUZ_PIN, NOTE_C3, VIOLATION_BLINK_PERIOD);
    delay(VIOLATION_BLINK_PERIOD);
    LEDbuffer = positionBuffer | startPattern[violator];
    updateLEDpins();
    tone(BUZ_PIN, NOTE_G2, VIOLATION_BLINK_PERIOD);
    delay(VIOLATION_BLINK_PERIOD); 
  }  
}

byte awaitingInput(byte who) {
  int input = who;
  if(who == ANYONE)
  {
    LEDbuffer = 0b10000001;
    updateLEDpins();
    while(!btnState[LEFT] && !btnState[RIGHT])
      delay(10);
    if(btnState[LEFT])
      input = LEFT;
    else
      input = RIGHT;
  }
  else
    while (!btnState[who])
      blinkLEDsAwaiting(who);
  return input;
}


void loop() {
  int hitCount;
  score[LEFT] = 0;
  score[RIGHT] = 0;
  scoreBuffer = 0;
  startingSide = ANYONE;
  sound(BOOTUP);

GAME_START:
  resetFlags();
  startingSide = awaitingInput(startingSide);
  sound(startingSide);
  positionBuffer = startPattern[startingSide];
  LEDbuffer = positionBuffer;
  updateLEDpins();
  
  delay(gameDelay);
  resetFlags();
  hitCount = 0;
  
  if(startingSide == RIGHT)
    goto RIGHT_CONTINUE;

  while(1)
  {

    do
    {
      positionBuffer >>= 1;
      LEDbuffer = positionBuffer;
      updateLEDpins();
      delay(gameDelay);
      checkViolation();
      if(violator != NO_ONE)
        goto VIOLATION_EVALUATION;
      else
        resetFlags();
    }
    while(positionBuffer > 0b00000001);
    sound(RIGHT);
    
RIGHT_CONTINUE:
    do
    {
      positionBuffer <<= 1;
      LEDbuffer = positionBuffer;
      updateLEDpins();
      delay(gameDelay);
      checkViolation();
      if(violator != NO_ONE)
        goto VIOLATION_EVALUATION;
      else
        resetFlags();
    }
    while(positionBuffer < 0b10000000);
    sound(LEFT);

    hitCount++;
    if ((hitCount > INCREMENT_DELAY_HIT_TRESHOLD) && (gameDelay > 100))
      gameDelay -= DELAY_SUBTRAHEND;
  }

VIOLATION_EVALUATION:
  signalViolation();
  score[(violator+1)%2]++;
  updateScoreBuffer();
  if(score[(violator+1)%2] < MAX_SCORE)
  {
    startingSide = violator;
    goto GAME_START;
  }

  else
  {
    LEDbuffer = scoreBuffer;
    updateLEDpins();
    sound(WIN);
  }
}

void sound(char type) {
  if(type == BOOTUP)
  {
    tone(BUZ_PIN, NOTE_G4, 150);
    delay(100);
    tone(BUZ_PIN, NOTE_C5, 200);
    delay(200);
  }
  else if(type == LEFT)
    tone(BUZ_PIN, NOTE_C5, 150);
  else if(type == RIGHT)
    tone(BUZ_PIN, NOTE_G4, 150);
  else if(type == WIN)
  {
    tone(BUZ_PIN, NOTE_C5, 150);
    delay(150);
    tone(BUZ_PIN, NOTE_E5, 150);
    delay(150);
    tone(BUZ_PIN, NOTE_G5, 150);
    delay(150);
    tone(BUZ_PIN, NOTE_C6, 350);
    delay(400);
    tone(BUZ_PIN, NOTE_G5, 200);
    delay(150);
    tone(BUZ_PIN, NOTE_C6, 400);
    delay(400);
    delay(500);
  }
}

void resetFlags() {
  violator = NO_ONE;
  btnState[LEFT] = 0;
  btnState[RIGHT] = 0;
}

void checkViolation() {
  if((btnState[LEFT] && (positionBuffer < 0b10000000)) || (!btnState[LEFT] && (positionBuffer == 0b10000000)))
    violator = LEFT;
  if((btnState[RIGHT] && (positionBuffer > 0b00000001)) || (!btnState[RIGHT] && (positionBuffer == 0b00000001)))
    violator = RIGHT;
}

void updateScoreBuffer() {
  //bit 0 is the least significant
  for(byte i = N_OF_LEDS - 2; i > N_OF_LEDS - 2 - score[LEFT]; i--)
    scoreBuffer |= 0b00000001 << i;
  for(byte i = 1; i < 1 + score[RIGHT]; i++)
    scoreBuffer |= 0b00000001 << i;
}
