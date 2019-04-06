
#include <bluefruit.h>
#include <Servo.h>



// Received Byte Code Translation Table
// function                    char   int value
#define DELIM_CHAR             ('!')  // 33
#define LEFT_MOTOR_CHAR        ('L')  // 76
#define RIGHT_MOTOR_CHAR       ('R')  // 82
#define SPEEDNEG2_CHAR         ('A')  // 65
#define SPEEDNEG1_CHAR         ('B')  // 66
#define SPEED0_CHAR            ('C')  // 67
#define SPEED1_CHAR            ('D')  // 69
#define SPEED2_CHAR            ('E')  // 70
#define SPEED3_CHAR            ('F')  // 71

// SPEED TO MICROSECONDS CONVERSIONS (length of pulses send to ESC)
#define ARM_VAL            (1500) // arms  the ESC
#define SPEED_NEG2         (1000) // 7.52v, ~90 rpm reverse
#define SPEED_NEG1         (1375) // 6.47v, ~77 rpm reverse
#define SPEED_0            (1500) // 0v, 0 rpm
#define SPEED_1            (1610) // 5.50v, ~65 rpm forward
#define SPEED_2            (1690) // 6.45v, ~77 rpm forward
#define SPEED_3            (2000) // 7.52v, ~90 rpm forward


// Services
BLEUart bleuart;
BLEDis bledis;
//BLEBas blebas;

// Electronic Speed Controls
Servo L_ESC;
Servo R_ESC;
Servo* cur_ESC;

void setup() 
{
  L_ESC.attach(15);
  R_ESC.attach(7);
  
  Serial.begin(115200); // 115200 is the baud rate the serial monitor will run on
  Serial.println("TANK CONTROLLER STARTUP");
  Serial.println("------------------------------------\n");
  Bluefruit.begin();                   
  //Bluefruit.setTxPower(0);                 // Expected signal power level at 1 meter (defaults to 0 anyway)
  Bluefruit.setName("SL_FRUIT");          // name in advertisnig packet
  
  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service (our packet transport service)
  bleuart.begin();

  // Start BLE Battery Service
  /*blebas.begin();
  blebas.update(100); not ready for this yet*/

  // Set up Advertising Packet
  startAdv();
  
  // Arm the ESC's
  L_ESC.writeMicroseconds(ARM_VAL);
  R_ESC.writeMicroseconds(ARM_VAL);
  delay(2000);
}

/* Makes the bluetooth module on the peripheral (the tank) start sending out advertising packets for  
   a central device (my phone) to find and connect with */
void startAdv()
{
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  /* I think this flag just says that its in discover mode and not sending real data */
  Bluefruit.Advertising.addTxPower(); // actually inputs the TxPower from setTxPower to the device
  Bluefruit.Advertising.addService(bleuart); // bleuart will be our sending/receiving service
  Bluefruit.ScanResponse.addName();
  /* actually adds the device name. Doesn't fit in the advertising packet, so it's sent separately */
  
  //Bluefruit.Advertising.restartOnDisconnect(true); // start scanning again if disconnected (default = true)
  //Bluefruit.Advertising.setInterval(32, 244);    
  // in unit of 0.625 ms: 20 ms interval fast mode, 152.5 ms slow mode (default values are these)
  
  //Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode (default = 30 anyway)
  Bluefruit.Advertising.start();                
  /* if no arguments are given, start() defaults to start(timeout = 0)...
  0 = Don't stop advertising after n seconds */
  Serial.println("Bluetooth Advertising has begun...");
}

void loop() 
{
  // Wait for new data to arrive (polling)
  while (bleuart.available())
  {
    char c = bleuart.read();
    if (c != DELIM_CHAR) {break;} // ignore anything not beginning with '!'
    
    uint8_t fail = 0; // to break after switch statements
    c = bleuart.read();
    switch(c)
    {
    case LEFT_MOTOR_CHAR :
      Serial.print("LEFT MOTOR :");
      cur_ESC = &L_ESC;
      break;
    case RIGHT_MOTOR_CHAR :
      Serial.print("RIGHT MOTOR :");
      cur_ESC = &R_ESC;
      break;
    default :
      Serial.println("Received invalid char after !");
      Serial.print("The character was: ");
      Serial.println(c);
      fail = 1;
    }
    if (fail) {break;}
    
    c = bleuart.read();
    switch(c)
    {
    case SPEEDNEG2_CHAR :
      Serial.println("SPEED -2");
      cur_ESC->writeMicroseconds(SPEED_NEG2);
      break;
    case SPEEDNEG1_CHAR :
      Serial.println("SPEED -1");
      cur_ESC->writeMicroseconds(SPEED_NEG1);
      break;
    case SPEED0_CHAR :
      Serial.println("SPEED 0");
      cur_ESC->writeMicroseconds(SPEED_0);
      break;
    case SPEED1_CHAR :
      Serial.println("SPEED 1");
      cur_ESC->writeMicroseconds(SPEED_1);
      break;
    case SPEED2_CHAR :
      Serial.println("SPEED 2");
      cur_ESC->writeMicroseconds(SPEED_2);
      break;
    case SPEED3_CHAR :
      Serial.println("SPEED 3");
      cur_ESC->writeMicroseconds(SPEED_3);
      break;
    default: 
      Serial.println("Received invalid char after L/R");
      Serial.print("The character was: ");
      Serial.println(c);
    }
  }
}
