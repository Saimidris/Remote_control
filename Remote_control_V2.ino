/*



Notes:
- e5: LoRa Module
- Serial: Console/USB
- u8x8: Oled LCD (Exp. board)
- lcd: 4x20 LCD Display

*/

//Libraries
#include <Arduino.h>
#include <U8x8lib.h>  //LCD expension board
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_PCF8574.h>  //LCD remote control
#include <Wire.h>
#include "I2CKeyPad.h"  //Keypad

//Define pins


//Declare components
SoftwareSerial e5(7, 6);  //LoRa E5 module: e5(RX, TX)
//U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);     //LCD expension board
const uint8_t LCD_ADDRESS = 0x27;
LiquidCrystal_PCF8574 lcd(LCD_ADDRESS);  //LCD Address is (0x27)
const uint8_t KEYPAD_ADDRESS = 0x20;     //Keypad address (0x20)
I2CKeyPad keyPad(KEYPAD_ADDRESS);
const int buzzer = 0;  //buzzer to pin 0

//Declare variables
char keymap[19] = "123A456B789C*0#DNF";  // N = NoKey, F = Fail
static char recv_buf[512];
static int led = 13;
int counter = 0;
int water_sensor_value = 0;
int error;


//----------------------------------FUNCTIONS-------------------------------------
static int at_send_check_response(char *p_ack, int timeout_ms, char *p_cmd, ...) {
  int ch;
  int num = 0;
  int index = 0;
  int startMillis = 0;
  va_list args;
  memset(recv_buf, 0, sizeof(recv_buf));
  va_start(args, p_cmd);
  e5.print(p_cmd);
  Serial.printf(p_cmd, args);
  va_end(args);
  delay(200);
  startMillis = millis();

  if (p_ack == NULL) {
    Serial.println("Ack Null");
    return 0;
  }

  do {
    while (e5.available() > 0) {
      ch = e5.read();
      recv_buf[index++] = ch;
      Serial.print((char)ch);
      delay(2);
    }

    if (strstr(recv_buf, p_ack) != NULL) {
      Serial.println("Acknoledgement in buffer");
      return 1;
    }

  } while (millis() - startMillis < timeout_ms);
  return 0;
}

static void display_enter_code_and_code(char code[]) {
  //int ok = 0;
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("------SEND CODE-----");
  lcd.setCursor(0, 2);
  lcd.print("PLS. ENTER CODE:");
  lcd.setCursor(16, 2);
  lcd.print("XXXX");
  lcd.setCursor(0, 3);
  lcd.print("#:CORRECT  *:EXIT");
  //ok = 1;
  //return ok;
}

static void display_code(char code[]) {
  lcd.setCursor(16, 2);
  lcd.print(code);
}

/*
static int keypad_get_code()
{
  //Variables
  char code[] = "XXXX";
  int counter_digits_code = 1;

  //Update display
  display_enter_code_and_code(code);

  //Get character while the code is not 4 digits
  while (counter_digits_code <= 4)
  {
    delay(10);  //For stability

    Serial.println("Waiting for keypad input");     //DEBUG
    Serial.println(counter_digits_code);            //DEBUG

    char ch_test = keyPad.getChar();                  //DEBUG
    Serial.print("getChar= "); Serial.println(ch_test);            //DEBUG

    if (keyPad.isPressed())
    {
      Serial.println("Keypad is pressed");          //DEBUG      
      char ch = keyPad.getChar();
      Serial.println(ch);
      delay(50);
      
      //Update display
      display_enter_code_and_code(code);

      switch (ch) {
        case '*':
          //Exit, code 1001
          code[0] = {'1'};
          code[1] = {'0'};
          code[2] = {'0'};
          code[3] = {'1'};
          counter_digits_code = 4; //To exit the loop directly
          break;

        case '#':
          //Correct/go 1 back
          counter_digits_code--;
          break;

        default:
          //Check if the character is between 1 and 9 (Not A,B,C or D)
          if (ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9')
          {
            //Assigns the ch to the current pos in the code
            code[counter_digits_code] = ch;

            // Display code
            display_code(code);

            //Go to next position
            counter_digits_code++;
          }
      }
    }
  }

  //Convert code (string) to int
  String code_string = String(code);
  int code_int = code_string.toInt();

  //Validate
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CODE: ");  lcd.print(code_int);
  lcd.setCursor(0, 1);
  lcd.print("Proceed?");
  lcd.setCursor(4, 3);
  lcd.print("A: YES");
  lcd.setCursor(11, 3);
  lcd.print("B: NO");


  bool ok = false;
  while (ok == false)
  {
    if (keyPad.isPressed())
    {
      char ch = keyPad.getChar();
      Serial.println(ch);
      delay(50);

      switch (ch) {
        case 'B':
          //Exit, code 1001
          code[0] = '1';
          code[1] = '0';
          code[2] = '0';
          code[3] = '1';
          ok = true;
          break;

        case 'A':
          //Validate
          ok = true;
          break;
      }
    }
  }

  return code_int;
}
*/

static int keypad_get_code() {
  //Variables
  char code[] = "XXXX";
  int counter_digits_code = 1;
  char ch;

  //Update display
  display_enter_code_and_code(code);

  //Get character while the code is not 4 digits
  while (counter_digits_code <= 4) {
    delay(10);                            //For stability
    Serial.println(counter_digits_code);  //DEBUG

    //Wait for an input
    while ((ch = keyPad.getChar()) == 'N') {
      Serial.println("Waiting for keypad input");  //DEBUG
      delay(1);                                    // Just wait for a key
    }

    // Wait for the key to be released
    while (keyPad.getChar() != 'N') {
      delay(1);
    }

    beep();

    Serial.println("Keypad is pressed");  //DEBUG
    Serial.print("Pressed key: ");
    Serial.println(ch);
    delay(50);

    //Update display
    display_enter_code_and_code(code);

    switch (ch) {
      case '*':
        //Exit, code 1001
        code[0] = { '1' };
        code[1] = { '0' };
        code[2] = { '0' };
        code[3] = { '1' };
        counter_digits_code = 5;  //To exit the loop directly
        break;

      case '#':
        //Correct/go 1 back
        counter_digits_code--;
        break;

      default:
        //Check if the character is between 0 and 9 (Not A,B,C or D)
        if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
          //Assigns the ch to the current pos in the code (1st digit = Pos. 0)
          code[counter_digits_code - 1] = ch;

          // Display code
          display_code(code);

          //Go to next position
          counter_digits_code++;
        } else {
        }
    }
  }

  //Convert code (string) to int
  String code_string = String(code);
  int code_int = code_string.toInt();
  Serial.print("code_int: ");
  Serial.println(code_int);  //DEBUG


  switch (code_int) {
    case 1001:
      //Case of an exit code: return the code directly
      return code_int;
      break;
    default:
      //Otherwise, ask the user to validate or cancel
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CODE: ");
      lcd.print(code_int);
      lcd.setCursor(0, 1);
      lcd.print("Proceed?");
      lcd.setCursor(4, 3);
      lcd.print("A: YES");
      lcd.setCursor(11, 3);
      lcd.print("B: NO");

      bool ok = false;
      while (ok == false) {
        if (keyPad.isPressed()) {
          char ch = keyPad.getChar();
          Serial.println(ch);
          delay(50);

          switch (ch) {
            case 'B':
              Serial.print("ch: ");
              Serial.println(ch);        //DEBUG
              Serial.println("CASE B");  //DEBUG
              //Exit, code 1001
              code_int = 1001;
              ok = true;
              break;

            case 'A':
              Serial.print("ch: ");
              Serial.println(ch);        //DEBUG
              Serial.println("CASE A");  //DEBUG
              //Validate
              ok = true;
              break;

            default:
              Serial.print("ch: ");
              Serial.println(ch);              //DEBUG
              Serial.println("Default CASE");  //DEBUG
              //Exit, code 1001
              code_int = 1001;
              ok = true;
          }
        }
      }
      return code_int;
  }
}

static int setup_keypad() {
  int setup_kypd_ok = 2;
  Wire.setClock(400000);
  if (keyPad.begin() == false) {
    lcd.print("\nKeypad ERROR.\nPlease reboot.\n");
    setup_kypd_ok = 0;
    return setup_kypd_ok;
  }
  keyPad.loadKeyMap(keymap);
  setup_kypd_ok = 1;
  Serial.print("setup_kypd_ok: ");
  Serial.println(setup_kypd_ok);
  delay(5000);
  return setup_kypd_ok;
}

static int setup_LoRa_module() {
  //Test commands
  Serial.println("E5 LOCAL TEST \r\n");

  int ret = at_send_check_response("+AT: OK", 100, "AT\r\n");
  ret = at_send_check_response("+MODE: TEST", 1000, "AT+MODE=TEST\r\n");
  //ret = at_send_check_response("+UART: TIMEOUT, 1000", 1000, "AT+UART: TIMEOUT, 1000\r\n");

  Serial.println("E5 LOCAL TEST DONE \r\n");
  delay(200);
  return ret;
}

static int setup_LCD() {
  //Check var
  int ok = 0;

  Wire.begin();
  Wire.beginTransmission(LCD_ADDRESS);
  error = Wire.endTransmission(LCD_ADDRESS);
  Serial.print("Error: ");
  Serial.print(error);

  if (error == 0) {
    Serial.println(": LCD found.");
    lcd.begin(20, 4);  // initialize the lcd
    ok = 1;
  } else {
    Serial.println(": LCD not found.");
    ok = 0;
  }
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  return ok;
}

static int setup_remote_control() {
  delay(2000);

  int check_setup = 0;

  Wire.begin();

  //Setup LCD
  check_setup = setup_LCD();
  if (check_setup == 0) { Serial.println("LCD Setup failed"); }

  //Setup Buzzer pin mode as output
  pinMode(buzzer, OUTPUT);
  delay(500);

  //Display Infos
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("SETUP");
  lcd.setCursor(0, 2);
  lcd.print("Please wait...");

  //Init com E5
  e5.begin(9600);

  delay(500);

  //Setup LoRa Module
  check_setup = setup_LoRa_module();
  if (check_setup == 0) { Serial.println("LoRa Module Setup failed"); }

  //Setup Keypad
  check_setup = setup_keypad();
  if (check_setup == 0) { Serial.println("KeyPad Setup failed"); }

  if (check_setup == 0) {
    //setup_failed
    Serial.println("SETUP RC FAILED");
    lcd.home();
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("  SETUP RC FAILED   ");
  } else {
    //End of setup, turn LED on again
    Serial.println("SETUP RC DONE");
    lcd.home();
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("   SETUP RC DONE");
  }
  return check_setup;
}

static void main_menu() {
  //Variables
  int choice = 0;
  int emer_code_sent_ok = 0;
  int code_sent_ok = 0;
  int code = 0;

  //Display menu
  display_main_menu();

  //Get user's entry
  choice = keypad_get_value();

  switch (choice) {
    case 1:  //1: EMERGENCY SURFACING

      lcd.setBacklight(255);
      lcd.home();
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("   EMERGENCY CODE   ");
      lcd.setCursor(0, 2);
      lcd.print("  ...BEING SENT...");

      emer_code_sent_ok = send_code(2222);
      delay(500);

      if (emer_code_sent_ok == 1) {
        //Display confirmation on the LCD for 5s
        lcd.setBacklight(255);
        lcd.home();
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("   EMERGENCY CODE   ");
        lcd.setCursor(0, 2);
        lcd.print("        SENT        ");
        Serial.println("Emergency code sent");
        delay(2000);
      } else {
        //Display error message on the LCD for 5s
        lcd.setBacklight(255);
        lcd.home();
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("   EMERGENCY CODE   ");
        lcd.setCursor(0, 2);
        lcd.print("    SEND FAILED     ");
        Serial.println("Emergency code send failed!\r\n\r\n");
        delay(5000);
      }
      break;

    case 2:
      //2: SEND CODE

      //2.1. Get the code from the user
      code = keypad_get_code();
      Serial.print("code = keypad_get_code: ");
      Serial.println(code);  //DEBUG

      switch (code) {
        case 1001:
          //In case of an exit code, go directly back to the main menu
          break;

        default:
          //2.2. Send the code
          lcd.setBacklight(255);
          lcd.home();
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("       CODE");
          lcd.setCursor(0, 2);
          lcd.print("  ...BEING SENT...");

          code_sent_ok = send_code(code);
          delay(500);

          if (code_sent_ok == 1) {
            //Display confirmation on the LCD for 5s
            lcd.setBacklight(255);
            lcd.home();
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("      CODE SENT");
            Serial.println("Code sent");
            delay(2000);
          } else {
            //Display error message on the LCD for 5s
            lcd.setBacklight(255);
            lcd.home();
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("  CODE SEND FAILED");
            Serial.println("Code send failed!\r\n\r\n");
            delay(5000);
          }
      }
      break;

    default:  //Choice not in the options, does nothing
      {
      }
  }
  //Check water sensor values
}

static void check_confirmation(int code) {
  char cmd[128];
  int recv_code = 0000;
  int startMillis = 0;
  int ret = 0;
  int timeout_ms = 10 * 1000; //10s

  //Check the confirmation
  Serial.println("Checking the confirmation");
  startMillis = millis();
  sprintf(cmd, "AT+TEST=RXLRPKT");


  do {
    ret = at_send_check_response("+TEST: RXLRPKT", 5000, cmd);

    //Parse the code
    recv_code = recv_parse(recv_buf);
    Serial.print("Confirmation recv_code: ");
    Serial.println(recv_code);

    if (recv_code == code) {
      Serial.println("Confirmation received");
      //PRINT CODE ON LCD
    } else {
      Serial.print("Received confirmation code: ");
      Serial.println(recv_code);
      //PRINT CODE ON LCD
    }

    delay(10);
  } while ((recv_code != code) && (millis() - startMillis < timeout_ms));
  Serial.print("Received confirmation code: ");
  Serial.println(recv_code);

  if(recv_code == code)
  {
    display_confirmation(recv_code);
  }
  else {
    display_code_not_received(recv_code);
  }
}

static void display_code_not_received(int recv_code) {
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  NO CONFIRMATION");
  lcd.setCursor(0, 1);
  lcd.print("      RECEIVED");
  lcd.setCursor(0, 3);
  lcd.print("RECV. INFO:");
  lcd.setCursor(12, 3);
  lcd.print(recv_code);
  delay(3000);
}

static void display_confirmation(int code) {
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("    CONFIRMATION");
  lcd.setCursor(0, 2);
  lcd.print("      RECEIVED");
  
  delay(3000);
}

static int recv_parse(char *p_msg) {
  int code_int;

  //Check if there is a message to parse
  if (p_msg == NULL) {
    Serial.println("Received null");
    return 0;
  }
  char *p_start = NULL;
  char data[128];  // To hold the received bytes as characters

  int bytes_len = 0;
  p_start = strstr(p_msg, "RX");  //Locate substring "RX" in the message

  if (p_start && (1 == sscanf(p_start, "RX \"%s\"", &data))) {
    //If the message contains "RX" and the data can be found
    for (int i = 0; i < sizeof(data); i++) {
      if (int(data[i + 1]) == 0) {
        //Search the data string until the 0 at the end (find length)
        bytes_len = i;
        break;
      }
    }

    // Convert the characters to a byteArray
    int message_len = bytes_len / 2 + 1;
    byte out[message_len];
    auto getNum = [](char c) {
      return (c > '9') ? (c - 'A' + 10) : (c - '0');
    };
    for (int x = 0, y = 0; x < bytes_len; ++x, ++y)
      out[y] = (getNum(data[x++]) << 4) + getNum(data[x]);
    out[message_len] = '\0';

    // Print the received bytes
    Serial.print("Print the received bytes: ");
    for (int i = 0; i < sizeof(out) - 1; i++) {
      Serial.print(out[i], HEX);
      //Serial.print("-");
    }

    Serial.println("");
    char code[32] = "";
    array_to_string(out, 2, code);
    Serial.print("code: ");
    Serial.println(code);
    Serial.println("");

    //Convert code (string) to int
    String code_string = String(code);
    code_int = code_string.toInt();
    Serial.print("code_int: ");
    Serial.println(code_int);
    Serial.println("");
  }
  return code_int;
}

void array_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
    buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
  }
  //buffer[len*2] = '\0';
}

static int send_code(int code) {
  char cmd[128];
  sprintf(cmd, "AT+TEST=TXLRPKT,\"%d\"\r\n", code);
  int ret = at_send_check_response("+TEST: TXLRPKT", 5000, cmd);

  check_confirmation(code);

  delay(200);
  return ret;
}

static void display_main_menu() {
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("-----MAIN MENU------");
  lcd.setCursor(0, 2);
  lcd.print("1: EMERGENCY SURF.");
  lcd.setCursor(0, 3);
  lcd.print("2: SEND CODE");
}

static int keypad_get_value() {
  //This function gets a number between 0 and 9 from the keypad

  bool get_val = false;
  int val = 0;
  int startMillis = 0;
  startMillis = millis();
  int delay_ms = 20000;  //20s

  while ((get_val == false) && (millis() - startMillis < delay_ms)) {
    //Wait for the input until it gets a value OR the delay is expired
    if (keyPad.isPressed()) {
      char ch = keyPad.getChar();
      delay(50);

      if (ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
        //Convert the char. and assign the value
        String char_string = String(ch);
        int code_int = char_string.toInt();
        val = code_int;
        Serial.println(code_int);
        get_val = true;
      }
    }
    delay(500);
  }
  return val;
}

static void beep() {
  tone(buzzer, 1000);  // Send 1KHz sound signal...
  delay(50);           // ...for 1 sec
  noTone(buzzer);      // Stop sound...
}

//----------------------------------SETUP-------------------------------------
void setup(void) {
  int check_setup_rc = 0;

  //Initiate communication with the console
  Serial.begin(115200);
  delay(1000);
  Serial.println("BEGIN SETUP");

  //Turn off LED
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  check_setup_rc = setup_remote_control();

  //Display infos on console
  if (check_setup_rc == 0) {
    //setup failed, print on console
    Serial.print("check_setup_rc: ");
    Serial.println(check_setup_rc);
    Serial.println("SETUP FAILED");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("SETUP FAILED");
    lcd.setCursor(0, 2);
    lcd.print("PLEASE RESET");
  } else {
    //End of setup, turn LED on again
    Serial.println("SETUP DONE");
    digitalWrite(led, HIGH);
  }
}

//----------------------------------LOOP-------------------------------------
void loop(void) {
  //char cmd[128];
  //counter = counter + 1;

  //Get water sensor value
  //water_sensor_value = water_sensor_status();

  // Transmit HEX Value
  //sprintf(cmd, "AT+TEST=TXLRPKT,\"%d\"\r\n", water_sensor_value);
  //int ret = at_send_check_response("+TEST: TXLRPKT", 5000, cmd);
  //Serial.print("ret: "); Serial.println(ret);
  //if (ret)
  //Serial.println("Sent");
  //else
  //Serial.println("Send failed!\r\n\r\n");
  //delay(2000);

  main_menu();
}