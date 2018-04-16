#include "PinChangeInterrupt.h"
#include <avr/wdt.h>

//======== ACCESO O MOLINETE =======

//#define acceso
#define molinete

//===================================

#ifdef molinete
  #ifdef acceso
    "ERROR MAS DE UNA DEFINICION"
  #endif
  #else
    #ifdef acceso
    #else
    "ERROR NO HAY DEFINICION"
  #endif
#endif

//===================================

// PCB led

#define LED_PCB 13
#define TICK_MS 100

// Valid PinChangeInterrupt for RDIF Reader

#define RFID_1_D0 5
#define RFID_1_D1 4

#define RFID_2_D0 7
#define RFID_2_D1 6

// Wiegand parameters

#define MAX_BITS 26                      // Max number of bits 
#define WEIGAND_WAIT_TIME  4             //  Time to wait for another weigand pulse.

// Rele parameters

#define       RELE_WAIT_TIME     40      // Time to wait for rele close
unsigned char Rele_counter    =  0;      // countdown to close rele

// Serial parameters

#define SYSTEM_TIME_OUT        600       // Time to reboot system
unsigned int System_Time      =  SYSTEM_TIME_OUT;

// Variables

unsigned char Rfid_1_bits [MAX_BITS];        // stores all of the data bits
unsigned char Rfid_2_bits [MAX_BITS];        // stores all of the data bits

unsigned char BitCount1 = 0;                               // number of bits currently captured
unsigned char Wiegand_Counter1 = WEIGAND_WAIT_TIME;        // countdown until we assume there are no more bits

unsigned char BitCount2 = 0;                               // number of bits currently captured
unsigned char Wiegand_Counter2 = WEIGAND_WAIT_TIME;        // countdown until we assume there are no more bits

int           inByte = 0;                                  // byte for serial
unsigned char toogle_led = 0;                              // led

unsigned char Flag_Ticks = 0;                              //
unsigned char Flag_Ticks_Old = 0;                          //

//==============================================
#ifdef acceso

#define POWER_RS232       A0

#define BARRIER_1_UP      A1
#define BARRIER_1_DOWN    A2

#define PUSH_1_UP         3
#define PUSH_1_DOWN       2

#define BARRIER_2_UP      A3
#define BARRIER_2_DOWN    A5

#define PUSH_2_UP         11
#define PUSH_2_DOWN       10

//==============================================
#endif

#ifdef molinete
//==============================================
#define POWER_RS232      A1

#define OPTO_1 9      // Optocoupler input
#define OPTO_2 8      // Optocoupler input

#define LED_1 10      // Led Green / Red
#define LED_2 11      // Led Green / Red

#define CLAMP    A0   // Clamp Rele
#define RELE_AUX A4   // Auxiliar Rele

unsigned char a = 0, a_old = 0 , b_old = 0, b = 0;                    // encoder variables
unsigned char Turn_1 = 0, Turn_2 = 0, Turn_3 = 0, flagTurn = 0 ;      // molinete control variables
signed   char phi = 0;                                                // angle in encoder
//==============================================
#endif

void setup() {

  // Enable Watch Dog
  wdt_enable(WDTO_250MS);

  wdt_reset();
  
  // Configure Barriers

#ifdef acceso
  pinMode(BARRIER_1_UP, OUTPUT);
  digitalWrite(BARRIER_1_UP, 1);

  pinMode(BARRIER_1_DOWN, OUTPUT);
  digitalWrite(BARRIER_1_DOWN, 1);

  pinMode(BARRIER_2_UP, OUTPUT);
  digitalWrite(BARRIER_2_UP, 1);

  pinMode(BARRIER_2_DOWN, OUTPUT);
  digitalWrite(BARRIER_2_DOWN, 1);

  pinMode(PUSH_1_UP,   INPUT_PULLUP);
  pinMode(PUSH_1_DOWN, INPUT_PULLUP);

  pinMode(PUSH_2_UP,   INPUT_PULLUP);
  pinMode(PUSH_2_DOWN, INPUT_PULLUP);

#endif

#ifdef molinete

  // Powering RS232

  pinMode(POWER_RS232, OUTPUT);
  digitalWrite(POWER_RS232, 1);

  // Configure Reles
  pinMode(CLAMP, OUTPUT);
  digitalWrite(CLAMP, 0);
  pinMode(RELE_AUX, OUTPUT);
  digitalWrite(RELE_AUX, 0);

  pinMode(OPTO_1, INPUT_PULLUP);
  pinMode(OPTO_2, INPUT_PULLUP);

  // Configure Led's

  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, 1);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_2, 1);

#endif
  // Configure Led PCB

  pinMode(LED_PCB, OUTPUT);
  digitalWrite(LED_PCB, 1);

  // Set pin to input with a pullup, led to output

  pinMode(RFID_1_D0, INPUT_PULLUP);
  pinMode(RFID_1_D1, INPUT_PULLUP);
  pinMode(RFID_2_D0, INPUT_PULLUP);
  pinMode(RFID_2_D1, INPUT_PULLUP);

  wdt_reset();

  // initialize serial ports:
  Serial.begin(9600);
  SendStart();  // Send start message

  // Attach the new PinChangeInterrupt and enable event function below

  attachPCINT(digitalPinToPCINT(RFID_1_D0), ISR_interruptOnChange_1, FALLING);
  attachPCINT(digitalPinToPCINT(RFID_1_D1), ISR_interruptOnChange_1, FALLING);

  attachPCINT(digitalPinToPCINT(RFID_2_D0), ISR_interruptOnChange_2, FALLING);
  attachPCINT(digitalPinToPCINT(RFID_2_D1), ISR_interruptOnChange_2, FALLING);

#ifdef acceso

  attachPCINT(digitalPinToPCINT(PUSH_1_UP),   ISR_interruptOnChange_3, FALLING);
  attachPCINT(digitalPinToPCINT(PUSH_1_DOWN), ISR_interruptOnChange_3, FALLING);

  attachPCINT(digitalPinToPCINT(PUSH_2_UP),   ISR_interruptOnChange_4, FALLING);
  attachPCINT(digitalPinToPCINT(PUSH_2_DOWN), ISR_interruptOnChange_4, FALLING);
#endif
}

void ISR_interruptOnChange_1(void) {
  System_Time    =  SYSTEM_TIME_OUT;

  if (digitalRead(RFID_1_D0) == 0)
  {
    if ( BitCount1 < MAX_BITS )
    {
      Rfid_1_bits[BitCount1] = 0;
      BitCount1++;
      Wiegand_Counter1 = WEIGAND_WAIT_TIME;
    } else if (BitCount1 == MAX_BITS ) (Wiegand_Counter1 = 1);
  }

  if (digitalRead(RFID_1_D1) == 0)
  {
    if ( BitCount1 < MAX_BITS )
    {
      Rfid_1_bits[BitCount1] = 1;
      BitCount1++;
      Wiegand_Counter1 = WEIGAND_WAIT_TIME;
    } else if (BitCount1 == MAX_BITS ) (Wiegand_Counter1 = 1);
  }
}

void ISR_interruptOnChange_2(void) {
  System_Time    =  SYSTEM_TIME_OUT;
  if (digitalRead(RFID_2_D0) == 0)
  {
    if ( BitCount2 < MAX_BITS )
    {
      Rfid_2_bits[ BitCount2 ] = 0;
      BitCount2++;
      Wiegand_Counter2 = WEIGAND_WAIT_TIME;
    } else if (BitCount2 == MAX_BITS) (Wiegand_Counter2 = 1);
  }

  if (digitalRead(RFID_2_D1) == 0)
  {
    if ( BitCount2 < MAX_BITS )
    {
      Rfid_2_bits[ BitCount2] = 1;
      BitCount2++;
      Wiegand_Counter2 = WEIGAND_WAIT_TIME;
    } else if (BitCount2 == MAX_BITS) (Wiegand_Counter2 = 1);
  }
}
#ifdef acceso

void ISR_interruptOnChange_3(void) {
  System_Time    =  SYSTEM_TIME_OUT;

  if (digitalRead(PUSH_1_UP) == 0)
  {
    digitalWrite(BARRIER_1_UP   , 1);
    digitalWrite(BARRIER_1_DOWN , 0);

    Rele_counter = RELE_WAIT_TIME;

  } else if (digitalRead(PUSH_1_DOWN) == 0)
  {
    digitalWrite(BARRIER_1_UP   , 0);
    digitalWrite(BARRIER_1_DOWN , 1);

    Rele_counter = RELE_WAIT_TIME;
  }
}

void ISR_interruptOnChange_4(void) {
  System_Time    =  SYSTEM_TIME_OUT;

  if (digitalRead(PUSH_2_UP) == 0)
  {
    digitalWrite(BARRIER_2_UP   , 1);
    digitalWrite(BARRIER_2_DOWN , 0);

    Rele_counter = RELE_WAIT_TIME;

  } else if (digitalRead(PUSH_2_DOWN) == 0)
  {
    digitalWrite(BARRIER_2_UP   , 0);
    digitalWrite(BARRIER_2_DOWN , 1);

    Rele_counter = RELE_WAIT_TIME;
  }
}
#endif

unsigned char processData ( unsigned char databits[0], unsigned long * facilityCode , unsigned long * cardCode  ) {
  unsigned char i;
  unsigned char ParityEven = 0;
  unsigned char ParityOdd  = 0;

  // Data analysis
  for (i = 1; i < 9; i++)
  {
    *facilityCode <<= 1;
    *facilityCode |= databits[i];
  }
  for (i = 9; i < 25; i++)
  {
    *cardCode <<= 1;
    *cardCode |= databits[i];
  }

  // Check Parity
  for (i = 0; i < 13; i++) {
    if (databits[i] == 1) ParityEven ^= B00000001;
  }
  for (i = 13; i < 26; i++) {
    if (databits[i] == 1) ParityOdd  ^= B00000001;
  }

  if ( !ParityEven  && ParityOdd )
  {
    return 1 ;
  } else
  {
    return 0 ;
  }
}
#ifdef molinete

void processEncoder() {
  phi = 0;
  if ((a_old == 0) && (b_old == 0) && (a == 1) && (b == 0) )  {
    phi = 1 ;
  }
  if ((a_old == 1) && (b_old == 0) && (a == 1) && (b == 1) )  {
    phi = 2 ;
  }
  if ((a_old == 1) && (b_old == 1) && (a == 0) && (b == 1) )  {
    phi = 3 ;
  }
  if ((a_old == 0) && (b_old == 1) && (a == 0) && (b == 0) )  {
    phi = 4 ;
  }

  if ((a_old == 0) && (b_old == 0) && (a == 0) && (b == 1) )  {
    phi = -1 ;
  }
  if ((a_old == 0) && (b_old == 1) && (a == 1) && (b == 1) )  {
    phi = -2 ;
  }
  if ((a_old == 1) && (b_old == 1) && (a == 1) && (b == 0) )  {
    phi = -3 ;
  }
  if ((a_old == 1) && (b_old == 0) && (a == 0) && (b == 0) )  {
    phi = -4 ;
  }
}
#endif
#ifdef molinete

void processClamp() {
  if ( (Turn_1 == 0)  && ( Turn_2 == 0) ) {
    if  ( phi ==  1 ) {
      digitalWrite(CLAMP, 1);
    }
    if  ( phi == -1 ) {
      digitalWrite(CLAMP, 1);
    }
    if  ( phi ==  4 ) {
      digitalWrite(CLAMP, 0);
    }
    if  ( phi == -4 ) {
      digitalWrite(CLAMP, 0);
    }
  }

  if ( (Turn_1 == 1)  && ( Turn_2 == 0) ) {
    if  ( phi ==  2 ) {
      flagTurn  = 1;
    }
    if  ( phi == -1 ) {
      digitalWrite(CLAMP, 1);
    }
    if  ( phi ==  4 ) {
      digitalWrite(CLAMP, 0);
      if (flagTurn) {
        Turn_1 = 0 ;
        digitalWrite(LED_1, 1);
        flagTurn = 0;
      }
    }
    if  ( phi == -4 ) {
      digitalWrite(CLAMP, 0);
    }
  }


  if ( (Turn_1 == 0) && ( Turn_2 == 1) ) {
    if  ( phi == -2 ) {
      flagTurn  = 1;
    }
    if  ( phi ==  1 ) {
      digitalWrite(CLAMP, 1);
    }
    if  ( phi ==  4 ) {
      digitalWrite(CLAMP, 0);
    }
    if  ( phi == -4 ) {
      digitalWrite(CLAMP, 0);
      if (flagTurn) {
        Turn_2 = 0 ;
        flagTurn = 0;
        digitalWrite(LED_2, 1);
      }
    }
  }
}
#endif

void ProcessReader1() {

  unsigned long facilityCode = 0;      // decoded facility code
  unsigned long cardCode = 0;          // decoded card code

  if (BitCount1 == 26) {
    if (processData ( Rfid_1_bits, &facilityCode, &cardCode )) {
      Serial.write(1);
      Serial.write("1:");
      Serial.print(facilityCode);
      Serial.print(cardCode);
      Serial.write(2);
      SendCRLF();
    } else
    {
      Serial.write(1);
      Serial.write("1:");
      Serial.write("Error");
      Serial.write(2);
      SendCRLF();
    }
  }
  BitCount1 = 0;
}

void ProcessReader2 () {

  unsigned long facilityCode = 0;      // decoded facility code
  unsigned long cardCode = 0;          // decoded card code

  if (BitCount2 == 26) {
    if (processData ( Rfid_2_bits, &facilityCode, &cardCode ) ) {
      Serial.write(1);
      Serial.write("2:");
      Serial.print(facilityCode);
      Serial.print(cardCode);
      Serial.write(2);
      SendCRLF();
    } else {
      Serial.write(1);
      Serial.write("2:");
      Serial.write("Error");
      Serial.write(2);
      SendCRLF();
    }
  }
  BitCount2 = 0;
}

void SendCRLF() {
  Serial.write(10);
  Serial.write(13);
}

void SendEnableUart() {
  Serial.write(1);
  Serial.write("Enabling Uart");
  Serial.write(2);
  SendCRLF();
}

void SendDisableUart() {
  Serial.write(1);
  Serial.write("Disabling Uart");
  Serial.write(2);
  SendCRLF();
}

void SendRebootRs() {
  Serial.write(1);
  Serial.write("Rebooting interface Power");
  Serial.write(2);
  SendCRLF();
}

void SendRebootByTimeOut() {
  Serial.write(1);
  Serial.write("Rebooting by timed out");
  Serial.write(2);
  SendCRLF();
}

void SendReboot() {
  Serial.write(1);
  Serial.write("Rebooting system");
  Serial.write(2);
  SendCRLF();
}

void SendOK() {
  Serial.write(1);
  Serial.write("OK");
  Serial.write(2);
  SendCRLF();
}

void SendStart() {
  Serial.write(1);
#ifdef acceso
  Serial.write("Starting acceso");
#endif
#ifdef molinete
  Serial.write("Starting molinete");
#endif
  Serial.write(2);
  SendCRLF();
}

void SendACK() {
  Serial.write(1);
  Serial.write("ACK");
  Serial.write(2);
  SendCRLF();
}

void SendNAK() {
  Serial.write(1);
  Serial.write("NAK");
  Serial.write(2);
  SendCRLF();
}

void ProcessSerial() {
  static unsigned char flagData = 0;
  static unsigned char command = 0;

  toogle_led = toogle_led ^ 1;
  digitalWrite(LED_PCB, toogle_led);

  wdt_reset();            // Kick watch dog

  switch (inByte) {
    case 1:
      flagData = 1;
      break;
    case 2:
      if ( flagData ) {
        flagData = 0;
        if (command == '1') {
#ifdef molinete
          Turn_1 = 1;
          Turn_2 = 0;
          Turn_3 = 0;

          digitalWrite(LED_1, 0);
          digitalWrite(LED_2, 1);

          Rele_counter = RELE_WAIT_TIME;

          SendACK();
#endif
#ifdef acceso
          SendNAK();
#endif

        } else if (command == '2') {
          System_Time    =  SYSTEM_TIME_OUT;

#ifdef molinete
          Turn_2 = 1;
          Turn_1 = 0;
          Turn_3 = 0;

          digitalWrite(LED_2, 0);
          digitalWrite(LED_1, 1);

          Rele_counter = RELE_WAIT_TIME;

          SendACK();
#endif
#ifdef acceso
          SendNAK();
#endif
        } else if (command == '3') {
          System_Time    =  SYSTEM_TIME_OUT;

#ifdef molinete
          Turn_3 = 1;
          Turn_1 = 0;
          Turn_2 = 0;
          digitalWrite(RELE_AUX , 1);

          Rele_counter = RELE_WAIT_TIME;
          SendACK();
#endif
#ifdef acceso
          SendNAK();
#endif
        } else if (command == '4') {
          System_Time    =  SYSTEM_TIME_OUT;

#ifdef acceso
          digitalWrite(BARRIER_1_DOWN , 1);
          digitalWrite(BARRIER_1_UP   , 0);

          Rele_counter = RELE_WAIT_TIME;
          SendACK();
#endif
#ifdef molinete
          SendNAK();
#endif
        } else if (command == '5') {
          System_Time    =  SYSTEM_TIME_OUT;
#ifdef acceso
          digitalWrite(BARRIER_1_UP   , 1);
          digitalWrite(BARRIER_1_DOWN , 0);

          Rele_counter = RELE_WAIT_TIME;
          SendACK();
#endif
#ifdef molinete
          SendNAK();
#endif

        } else if (command == '6') {
          System_Time    =  SYSTEM_TIME_OUT;
#ifdef acceso
          digitalWrite(BARRIER_2_DOWN , 1);
          digitalWrite(BARRIER_2_UP   , 0);

          Rele_counter = RELE_WAIT_TIME;

          SendACK();
#endif
#ifdef molinete
          SendNAK();
#endif
        } else if (command == '7') {
          System_Time    =  SYSTEM_TIME_OUT;

#ifdef acceso
          digitalWrite(BARRIER_2_UP   , 1);
          digitalWrite(BARRIER_2_DOWN , 0);

          Rele_counter = RELE_WAIT_TIME;

          SendACK();
#endif
#ifdef molinete
          SendNAK();
#endif
        } else if (command == '8') {
          System_Time    =  SYSTEM_TIME_OUT;
          SendOK();

        } else if (command == '9') {
          System_Time    =  SYSTEM_TIME_OUT;
          Reset_Uart();

        } else if (command == 'A') {
          System_Time    =  SYSTEM_TIME_OUT;
          Reset_Power_Rs232();

        } else if (command == 'B') {
          System_Time    =  SYSTEM_TIME_OUT;
          Reset_System ();

        } else SendNAK();
      } else SendNAK();
      break;
    default:
      if (flagData) (command = inByte);
      break;
  }
}

void Reset_System() {
  SendReboot();                   // message
  delay(30);
  Serial.end();                   // stop the uart
  digitalWrite(POWER_RS232, 0);   // turn off the interfase
  while (1 == 1) {};
}

void Reset_Uart() {
  SendDisableUart();
  delay(30);
  Serial.end();
  Serial.begin(9600);
  SendEnableUart();
}

void Delay_1_Seg(void) {
  unsigned char i;
  for (i = 1; i < 100; i++)
  {
    wdt_reset();            // Kick watch dog
    delay (10);
  }
}

void Reset_Power_Rs232() {
  SendRebootRs();
  delay(30);
  Serial.end();
  digitalWrite(POWER_RS232, 0);
  Delay_1_Seg();
  digitalWrite(POWER_RS232, 1);
  delay(30);
  Serial.begin(9600);
}

void Check_Serial(void) {
  // Read the serial
  if (Serial.available()) {
    inByte = Serial.read();
    if (inByte) {                               // si es cero no lo proceso
      ProcessSerial();
    }
  }
}

void Check_TimeOut_Reles(void) {
  if ( Rele_counter == 0)
  {
#ifdef molinete
    Turn_1 = 0 ;
    Turn_2 = 0 ;
    Turn_3 = 0;

    digitalWrite(LED_1, 1);
    digitalWrite(LED_2, 1);
    digitalWrite(RELE_AUX, 0);
#endif


#ifdef acceso
    digitalWrite(BARRIER_1_UP   , 1);
    digitalWrite(BARRIER_1_DOWN , 1);
    digitalWrite(BARRIER_2_UP   , 1);
    digitalWrite(BARRIER_2_DOWN , 1);
#endif
  }
}

void Check_TimeOut_System(void) {
  // Serial ?

  if ( System_Time == 0 ) {

    SendRebootByTimeOut();
    digitalWrite(POWER_RS232, 0);
    while (1 == 1) {};
  }
}

void Check_TimeOut_Wiegand(void) {
  if ( Wiegand_Counter1 == 0 ) ProcessReader1();
  if ( Wiegand_Counter2 == 0 ) ProcessReader2();
}

void Check_Encoder() {

#ifdef molinete
  // Is there any change in encoder?
  a_old = a;
  a = digitalRead(OPTO_1);

  b_old = b;
  b = digitalRead(OPTO_2);

  if ((a != a_old ) || ( b != b_old ))
  {
    processEncoder();
    processClamp();
  }
#endif
}

void Ticks() {

  if ( (millis() % (TICK_MS * 2)) < TICK_MS ) {
    Flag_Ticks = 0;
  } else {
    Flag_Ticks = 1;
  }

  if (Flag_Ticks != Flag_Ticks_Old) {
    Flag_Ticks_Old = Flag_Ticks;
    // =============== Kick  ==============
    
    if (Rele_counter) {
      Rele_counter--;
    }

    if (System_Time) {
      System_Time--;
    }

    // Reader 1?
    if (Wiegand_Counter1) {
      Wiegand_Counter1--;
    }

    // Reader 2?
    if (Wiegand_Counter2) {
      Wiegand_Counter2--;
    }

    // ====================================
  }
}

void loop() {
  wdt_reset();            // Kick watch dog

  Ticks();

  Check_TimeOut_System();

  Check_TimeOut_Reles();

  wdt_reset();

  Check_TimeOut_Wiegand();

  Check_Serial();

  wdt_reset();  

#ifdef molinete
  Check_Encoder();
#endif
}

