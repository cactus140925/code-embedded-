#include <Bluepad32.h>
#include<ESP32Servo.h>
//them cac output dk
int led = 23;
int in1 = 25;
int in2 = 26;
int in3 = 27;
int in4 = 14;
int ENApin =15;
int ENBpin=18;
int relayPin = 21;
int xServoPin = 2;
int yServoPin =32;
int zServoPin = 13;
int zServoPos = 90;  // Bắt đầu từ vị trí trung tâm 
int tServoPin = 22;
int mServoPin =19;
int tServoPos = 0;
int mServoPos = 0;
bool relayState = false;
bool relayState2 = false;
bool rightState = false;
bool leftState = false;
Servo xServo;
Servo yServo;
Servo zServo;
Servo tServo;
Servo mServo;
#define BUTTON_TRIANGLE 0x0200
#define BUTTON_SQUARE 0X0020
#define BUTTON_CIRCLE 0x0100
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }
    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
        "misc: 5430x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
        ctl->index(),
        ctl->dpad(),
        ctl->buttons(),
        ctl->axisX(),
        ctl->axisY(),
        ctl->axisRX(),
        ctl->axisRY(),
        ctl->brake(),
        ctl->throttle(),
        ctl->miscButtons(),
        ctl->gyroX(),
        ctl->gyroY(),
        ctl->gyroZ(),
        ctl->accelX(),
        ctl->accelY(),
        ctl->accelZ()
    );
    if (ctl->dpad() == 0x01) {
        digitalWrite(led, 1);
        Serial.println("led on");
    } else {
        digitalWrite(led, 0);
    }
}

/*void controlCar(ControllerPtr ctl) {
    int axisX = ctl->axisX();
    int axisY = ctl->axisY();
    int motorSpeed = map(abs(axisY), 50, 512, 120, 255); // Ánh xạ tốc độ từ 70 đến 255 dựa trên axisY
    
    if (axisY < -50) {
        // Tiến: Cả hai động cơ chạy cùng chiều
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        analogWrite(ENApin, motorSpeed); // Tốc độ động cơ trái
        analogWrite(ENBpin, motorSpeed); // Tốc độ động cơ phải
        Serial.println("Car moving forward");
    } else if (axisY > 50) {
        // Lùi: Cả hai động cơ chạy ngược chiều
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
        analogWrite(ENApin, motorSpeed); // Tốc độ động cơ trái
        analogWrite(ENBpin, motorSpeed); // Tốc độ động cơ phải
        Serial.println("Car moving backward");
    } else if (axisX > 50) {
        // Rẽ phải: Động cơ trái chạy, động cơ phải dừng
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
        analogWrite(ENApin, motorSpeed); // Tốc độ động cơ trái
        analogWrite(ENBpin, 0);          // Động cơ phải dừng
        Serial.println("Car turning right");
    } else if (axisX < -50) {
        // Rẽ trái: Động cơ phải chạy, động cơ trái dừng
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        analogWrite(ENApin, 0);          // Động cơ trái dừng
        analogWrite(ENBpin, motorSpeed); // Tốc độ động cơ phải
        Serial.println("Car turning left");
    } else {
        // Dừng
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
        analogWrite(ENApin, 0);
        analogWrite(ENBpin, 0);
        Serial.println("Car stopped");
    }
}*/

void controlCar(ControllerPtr ctl) {
    int axisX = ctl->axisX();
    int axisY = ctl->axisY();

    int baseSpeed = map(abs(axisY), 50, 512, 130, 255); // tăng tốc cơ bản
    int turnAdjust = map(abs(axisX), 0, 512, 0, baseSpeed / 4); // giảm ít hơn -> rẽ mạnh hơn

    if (abs(axisX) < 50 && abs(axisY) < 50) {
        // Dừng
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
        analogWrite(ENApin, 0);
        analogWrite(ENBpin, 0);
        return;
    }

    if (axisY < -50) {
        // ---- TIẾN ----
        if (axisX > 50) {
            // Tiến phải
            digitalWrite(in1, HIGH);
            digitalWrite(in2, LOW);
            digitalWrite(in3, HIGH);
            digitalWrite(in4, LOW);
            analogWrite(ENApin, constrain(baseSpeed + turnAdjust, 0, 255)); // trái nhanh hơn
            analogWrite(ENBpin, constrain(baseSpeed - turnAdjust * 1.5, 0, 255)); // phải chậm hơn nhiều
            Serial.println("Car moving forward-right FAST");
        } else if (axisX < -50) {
            // Tiến trái
            digitalWrite(in1, HIGH);
            digitalWrite(in2, LOW);
            digitalWrite(in3, HIGH);
            digitalWrite(in4, LOW);
            analogWrite(ENApin, constrain(baseSpeed - turnAdjust * 1.5, 0, 255)); // trái chậm hơn nhiều
            analogWrite(ENBpin, constrain(baseSpeed + turnAdjust, 0, 255)); // phải nhanh hơn
            Serial.println("Car moving forward-left FAST");
        } else {
            // Tiến thẳng
            digitalWrite(in1, HIGH);
            digitalWrite(in2, LOW);
            digitalWrite(in3, HIGH);
            digitalWrite(in4, LOW);
            analogWrite(ENApin, baseSpeed);
            analogWrite(ENBpin, baseSpeed);
            Serial.println("Car moving forward");
        }
    } else if (axisY > 50) {
        // ---- LÙI ----
        if (axisX > 50) {
            // Lùi phải
            digitalWrite(in1, LOW);
            digitalWrite(in2, HIGH);
            digitalWrite(in3, LOW);
            digitalWrite(in4, HIGH);
            analogWrite(ENApin, constrain(baseSpeed + turnAdjust, 0, 255)); // trái nhanh hơn
            analogWrite(ENBpin, constrain(baseSpeed - turnAdjust * 1.5, 0, 255)); // phải chậm hơn nhiều
            Serial.println("Car moving backward-right FAST");
        } else if (axisX < -50) {
            // Lùi trái
            digitalWrite(in1, LOW);
            digitalWrite(in2, HIGH);
            digitalWrite(in3, LOW);
            digitalWrite(in4, HIGH);
            analogWrite(ENApin, constrain(baseSpeed - turnAdjust * 1.5, 0, 255)); // trái chậm hơn nhiều
            analogWrite(ENBpin, constrain(baseSpeed + turnAdjust, 0, 255)); // phải nhanh hơn
            Serial.println("Car moving backward-left FAST");
        } else {
            // Lùi thẳng
            digitalWrite(in1, LOW);
digitalWrite(in2, HIGH);
            digitalWrite(in3, LOW);
            digitalWrite(in4, HIGH);
            analogWrite(ENApin, baseSpeed);
            analogWrite(ENBpin, baseSpeed);
            Serial.println("Car moving backward");
        }
    } else if (axisX > 50) {
        // Rẽ phải tại chỗ
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
        analogWrite(ENApin, baseSpeed);
        analogWrite(ENBpin, baseSpeed);
        Serial.println("Car turning right");
    } else if (axisX < -50) {
        // Rẽ trái tại chỗ
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        analogWrite(ENApin, baseSpeed);
        analogWrite(ENBpin, baseSpeed);
        Serial.println("Car turning left");
    }
}

void dumpMouse(ControllerPtr ctl) {
    Serial.printf("idx=%d, buttons: 0x%04x, scrollWheel=0x%04x, delta X: %4d, delta Y: %4d\n",
                   ctl->index(),
                   ctl->buttons(),
                   ctl->scrollWheel(),
                   ctl->deltaX(),
                   ctl->deltaY()
    );
}

void dumpKeyboard(ControllerPtr ctl) {
    static const char* key_names[] = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
        "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        "Enter", "Escape", "Backspace", "Tab", "Spacebar", "Underscore", "Equal", "OpenBracket", "CloseBracket",
        "Backslash", "Tilde", "SemiColon", "Quote", "GraveAccent", "Comma", "Dot", "Slash", "CapsLock",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
        "PrintScreen", "ScrollLock", "Pause", "Insert", "Home", "PageUp", "Delete", "End", "PageDown",
        "RightArrow", "LeftArrow", "DownArrow", "UpArrow",
    };
    static const char* modifier_names[] = {
        "Left Control", "Left Shift", "Left Alt", "Left Meta",
        "Right Control", "Right Shift", "Right Alt", "Right Meta",
    };
    Serial.printf("idx=%d, Pressed keys: ", ctl->index());
    for (int key = Keyboard_A; key <= Keyboard_UpArrow; key++) {
        if (ctl->isKeyPressed(static_cast<KeyboardKey>(key))) {
            const char* keyName = key_names[key-4];
            Serial.printf("%s,", keyName);
        }
    }
    for (int key = Keyboard_LeftControl; key <= Keyboard_RightMeta; key++) {
        if (ctl->isKeyPressed(static_cast<KeyboardKey>(key))) {
            const char* keyName = modifier_names[key-0xe0];
            Serial.printf("%s,", keyName);
        }
    }
    Console.printf("\n");
}

void dumpBalanceBoard(ControllerPtr ctl) {
    Serial.printf("idx=%d, TL=%u, TR=%u, BL=%u, BR=%u, temperature=%d\n",
                   ctl->index(),
                   ctl->topLeft(),
                   ctl->topRight(),
                   ctl->bottomLeft(),
                   ctl->bottomRight(),
                   ctl->temperature()
    );
}

void controlRelay(ControllerPtr ctl){
    static bool lastTriangleState = false;
    bool currentTriangleState = (ctl->buttons() & BUTTON_TRIANGLE);

    // Chỉ khi nhấn mới toggle relay
    if (currentTriangleState && !lastTriangleState) {
        relayState = !relayState;
        digitalWrite(relayPin, relayState);
        Serial.printf("Relay turned %s\n", relayState ? "ON" : "OFF");
    }

    // Cập nhật trạng thái nút lần trước
    lastTriangleState = currentTriangleState;
}
void controlShootServo(ControllerPtr ctl) {
    static bool lastCircleState = false;
    bool currentCircleState = (ctl->buttons() & BUTTON_CIRCLE);
    if (currentCircleState && !lastCircleState) {
        tServoPos = (tServoPos == 180) ? 0 : 180;
        tServo.write(tServoPos);
        Serial.printf("T Servo RIGHT → %d°\n", tServoPos);
    }
    lastCircleState = currentCircleState;
}
void controlMServo(ControllerPtr ctl) {
    static bool lastSquareState = false;
    bool currentSquareState = (ctl->buttons() & BUTTON_SQUARE);
    if (currentSquareState && !lastSquareState) {
        mServoPos = (mServoPos == 180) ? 0 : 180;
        mServo.write(mServoPos);
    }
    lastSquareState = currentSquareState;
}
int servoXPos = 90;  // Góc ban đầu 90°
int servoYPos = 90;

void controlServos(ControllerPtr ctl) {
    int axisX = ctl->axisRX(); // Joystick phải trục X
    int axisY = ctl->axisRY(); // Joystick phải trục Y

    // ===== Trục X: sang phải -> tăng góc, sang trái -> giảm góc =====
    if (axisX > 200) { // sang phải
        servoXPos += 7;
        if (servoXPos > 180) servoXPos = 180;
    } 
    else if (axisX < -200) { // sang trái


        servoXPos -= 7;
        if (servoXPos < 0) servoXPos = 0;
    }

    // ===== Trục Y: lên -> tăng góc, xuống -> giảm góc =====
    if (axisY < -200) { // lên trên
        servoYPos += 7;
        if (servoYPos > 180) servoYPos = 180;
    } 
    else if (axisY > 200) { // xuống dưới
        servoYPos -= 7;
        if (servoYPos < 0) servoYPos = 0;
    }

    // ===== Trục Z: nút trái/phải PS3 =====
    static unsigned long lastMove = 0;
    unsigned long now = millis();

    bool leftPressed = (ctl->buttons() & 0x0010);   // Nút trái PS3
    bool rightPressed = (ctl->buttons() & 0x0008);  // Nút phải PS3

    if (now - lastMove > 120) { // Giới hạn tốc độ xoay
        if (leftPressed) {
            zServoPos -= 7;
            if (zServoPos < 0) zServoPos = 0;
            Serial.printf("⬅️ Z Servo LEFT → %d°\n", zServoPos);
        }
        if (rightPressed) {
            zServoPos += 7;
            if (zServoPos > 180) zServoPos = 180;
            Serial.printf("➡️ Z Servo RIGHT → %d°\n", zServoPos);
        }
        lastMove = now;
    }

    // ===== Ghi góc ra servo =====
    xServo.write(servoXPos);
    yServo.write(servoYPos);
    zServo.write(zServoPos);

    Serial.printf("🎮 X=%d | Y=%d → XServo=%d° | YServo=%d° | ZServo=%d°\n",
                  axisX, axisY, servoXPos, servoYPos, zServoPos);

    delay(10); // Mượt hơn, tránh giật
}

void processGamepad(ControllerPtr ctl) {
    if (ctl->b()) {
        static int led = 0;
        led++;
        ctl->setPlayerLEDs(led & 0x0f);
    }

    if (ctl->x()) {
        ctl->playDualRumble(0, 250, 0x80, 0x40);
    }


   controlServos(ctl);
    dumpGamepad(ctl);
    controlCar(ctl); // Thêm điều khiển xe
    controlRelay(ctl);
  controlShootServo(ctl);
  controlMServo(ctl);

}
void processMouse(ControllerPtr ctl) {
    if (ctl->scrollWheel() > 0) {
    } else if (ctl->scrollWheel() < 0) {
    }
    dumpMouse(ctl);
}

void processKeyboard(ControllerPtr ctl) {
    if (!ctl->isAnyKeyPressed()) return;
    if (ctl->isKeyPressed(Keyboard_A)) Serial.println("Key 'A' pressed");
    if (ctl->isKeyPressed(Keyboard_LeftShift)) Serial.println("Key 'LEFT SHIFT' pressed");
    if (ctl->isKeyPressed(Keyboard_LeftArrow)) Serial.println("Key 'Left Arrow' pressed");
    dumpKeyboard(ctl);
}

void processBalanceBoard(ControllerPtr ctl) {
    if (ctl->topLeft() > 10000) {
    }
    dumpBalanceBoard(ctl);
}

void processControllers() {
    for (auto myController : myControllers) {
        if (  myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else if (myController->isMouse()) {
                processMouse(myController);
            } else if (myController->isKeyboard()) {
                processKeyboard(myController);
            } else if (myController->isBalanceBoard()) {
                processBalanceBoard(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
    }
}

// Arduino setup function. Runs in CPU 1
void setup() {
    pinMode(led, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    pinMode(ENApin, OUTPUT);
    pinMode(ENBpin, OUTPUT);
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);  // Relay ban đầu tắt
    xServo.attach(xServoPin);
    yServo.attach(yServoPin);
    zServo.attach(zServoPin);
    tServo.attach(tServoPin);
    mServo.attach(mServoPin);
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.enableVirtualDevice(false);
}

// Arduino loop function. Runs in CPU 1.
void loop() {
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();
    delay(150);
}
