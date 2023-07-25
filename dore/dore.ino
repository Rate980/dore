#define LED 25
#define SW0 15
#define SW1 13
#define SW2 12
#define SERVO_PIN 27
#define SERVO_CHANEL 0
#define SERVO_FREQ 50
#define SERVO_BIT_FREQ 8

#include <ESP32Servo.h>

Servo servo;

void setup()
{
    Serial.begin(115200);
    pinMode(LED, OUTPUT);
    pinMode(SW0, INPUT_PULLDOWN);
    pinMode(SW1, INPUT_PULLDOWN);
    pinMode(SW2, INPUT_PULLDOWN);

    // pinMode(SERVO_PIN, OUTPUT);
    // ledcSetup(SERVO_CHANEL, SERVO_FREQ, SERVO_BIT_FREQ);
    // ledcAttachPin(SERVO_PIN, SERVO_CHANEL);
    servo.attach(SERVO_PIN);
    servo.write(90);
    xTaskCreatePinnedToCore(servoTask, "servoTask", 4096, NULL, 1, NULL, 0);
}
bool swState;
bool swStatePrev = false;
void loop()
{
    vTaskDelete(NULL);
    swState = digitalRead(SW0);
    if (swState != swStatePrev)
    {
        Serial.println(swState ? "ON" : "OFF");
        digitalWrite(LED, swState);
    }
    swStatePrev = swState;
    delay(1);
}

// サーボモーターを回すタスク
void servoTask(void *_)
{
    auto sw1old = digitalRead(SW1);
    auto sw2old = digitalRead(SW2);
    auto state = true;
    servo.write(90);
    while (true)
    {
        servo.write(0);
        Serial.println("0");
        delay(2000);
        servo.write(90);
        Serial.println("90");
        delay(2000);
        servo.write(180);
        Serial.println("180");
        delay(2000);
        servo.write(90);
        Serial.println("90");
        delay(2000);
        // auto sw1 = digitalRead(SW1);
        // if (sw1 != sw1old && sw1 == HIGH)
        // {
        //     if (state)
        //     {
        //         servo.write(180);
        //     }
        //     else
        //     {
        //         servo.write(90);
        //     }
        //     state = !state;
        // }
        // sw1old = sw1;

        // auto sw2 = digitalRead(SW2);
        // if (sw1 != sw1old && sw1 == HIGH)
        // {
        //     servo.write(90);
        //     if (hi == 0)
        //     {
        //         hi = 180;
        //     }
        //     else
        //     {
        //         hi = 0;
        //     }
        //     Serial.println(hi);
        // }
        // if (sw2 == HIGH)
        // {
        //     servo.write(hi);
        // }
        // else
        // {
        //     servo.write(90);
        // }
        delay(1);
    }
}
