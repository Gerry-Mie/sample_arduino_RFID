#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 5

const byte RGB_RED_PIN = 4;
const byte RGB_GREEN_PIN = 3;
const byte RGB_BLUE_PIN = 2;
const byte BUTTON_PIN = 7;

const byte MODE_AUTH = 0, MODE_REGISTER = 2, MODE_VERIFY = 1;
bool verified = false;
byte mode = MODE_AUTH;

String UIDs[10] = {"", "", "", "", "", "", "", "", "", ""};


MFRC522 rfid(SS_PIN, RST_PIN);

// functions
String useRFID();
void setRGB(byte r, byte g, byte b), signalError(), signalSuccess(), useButton();


void setup() {
    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_GREEN_PIN, OUTPUT);
    pinMode(RGB_BLUE_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.begin(9600);
    SPI.begin(); // init SPI bus
    rfid.PCD_Init(); // init MFRC522
    Serial.println("Tap RFID/NFC Tag on reader");

    if (!UIDs[0].length()) {
        mode = MODE_REGISTER;
    }
}

void loop() {


    String code = useRFID();
    if (code.length()) Serial.println(code);

    if (code.length()) {
        if (mode == MODE_REGISTER) {
            if (UIDs[9].length()) {
                signalError();
            }else {
                for (int i = 0; i < 10; ++i) {
                    if (code == UIDs[i]) {
                        signalError();
                        break;
                    }

                    if (!UIDs[i].length()) {
                        UIDs[i] = code;
                        signalSuccess();
                        break;
                    }
                }
            }
        } else if (mode == MODE_AUTH) {
            bool isAuthenticated = false;
            for (int i = 0; i < 10; ++i) {
                if (code == UIDs[i]) {
                    signalSuccess();
                    isAuthenticated = true;
                    break;
                }
            }
            if (!isAuthenticated) {
                signalError();
            }
        }
        else if (mode == MODE_VERIFY) {
            bool isSuccess = false;
            for (int i = 0; i < 10; ++i) {
                if (code == UIDs[i]) {
                    mode = MODE_REGISTER;
                    isSuccess = true;
                    break;
                }
            }
            if (!isSuccess) {
                signalError();
            }
        }
    }

    useButton();

    if (mode == MODE_REGISTER) {
        setRGB(0, 0, 255);
    } else if (mode == MODE_VERIFY) {
        setRGB(0, 0, 0);
        delay(100);
        setRGB(0, 255, 0);
    } else {
        setRGB(0, 0, 0);
    }

}

String useRFID() {
    String content = "";
    if (rfid.PICC_IsNewCardPresent()) { // new tag is available
        if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
            for (int i = 0; i < rfid.uid.size; i++) {
                content += rfid.uid.uidByte[i] < 0x10 ? " 0" : " ";
                content += String(rfid.uid.uidByte[i], HEX);
            }
            rfid.PICC_HaltA(); // halt PICC
            rfid.PCD_StopCrypto1(); // stop encryption on PCD
        }
    }
    return content;
}

void setRGB(byte r, byte g, byte b) {
    analogWrite(RGB_RED_PIN, r);
    analogWrite(RGB_GREEN_PIN, g);
    analogWrite(RGB_BLUE_PIN, b);
}

void signalError() {
    setRGB(255, 0, 0);
    delay(500);
}

void signalSuccess() {
    setRGB(0, 255, 0);
    delay(500);
}

int lastState = HIGH; // the previous state from the input pin
int currentState;

void useButton() {
    currentState = digitalRead(BUTTON_PIN);
    if(lastState == LOW && currentState == HIGH) {
        if (mode == MODE_REGISTER || mode == MODE_VERIFY) {
            mode = MODE_AUTH;
        } else {
            mode = MODE_VERIFY;
        }
    }
    lastState = currentState;
}

