#define LED 25
#define SW0 15
#define SW1 13
#define SW2 12

#define WINDOW0 25
#define WINDOW1 26
#define WINDOW2 15
#define WINDOW3 14

#define WINDOW_LENGTH 4

#define SERVO_PIN 27
#define SERVO_CHANEL 0
#define SERVO_FREQ 50
#define SERVO_BIT_FREQ 8

#define SERVO_LOCK 90
#define SERVO_UNLOCK 180

#define WiFI_SSID "maruyama"
#define WIFI_PASS "marufuck"
#define ROOM "3601"

#define URL(path) ("https://sora-b23da-default-rtdb.firebaseio.com/" ROOM "/" path ".json")

#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

typedef std::pair<uint8_t, bool> WindowMessage;

Servo servo;

bool servoState = false;
WiFiMulti wifiMulti;

uint8_t windows[] = {WINDOW0, WINDOW1, WINDOW2, WINDOW3};

QueueHandle_t uploaderQueue;

TaskHandle_t uploaders[1];

void setup()
{
    Serial.begin(115200);
    pinMode(LED, OUTPUT);
    pinMode(SW0, INPUT_PULLDOWN);
    pinMode(SW1, INPUT_PULLDOWN);
    pinMode(SW2, INPUT_PULLDOWN);
    for (auto pin : windows)
    {
        pinMode(pin, INPUT_PULLDOWN);
    }

    // pinMode(SERVO_PIN, OUTPUT);
    // ledcSetup(SERVO_CHANEL, SERVO_FREQ, SERVO_BIT_FREQ);
    // ledcAttachPin(SERVO_PIN, SERVO_CHANEL);
    servo.attach(SERVO_PIN);
    servo.write(90);
    wifiMulti.addAP(WiFI_SSID, WIFI_PASS);

    uploaderQueue = xQueueCreate(8, sizeof(WindowMessage));
    while (!Serial)
    {
        delay(1);
    }
    Serial.println("wait wifi");
    while (!wifiMulti.run() == WL_CONNECTED)
    {
        delay(1);
    }
    Serial.println("wifi connecting");
    xTaskCreatePinnedToCore(servoTask, "servoTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(watchDore, "watchDore", 8192, NULL, 1, NULL, 1);
    // xTaskCreatePinnedToCore(uploader, "uploader0", 8192, NULL, 1, &uploaders[0], 0);
    // xTaskCreatePinnedToCore(uploader, "uploader1", 8192, NULL, 1, &uploaders[1], 0);
    // xTaskCreatePinnedToCore(window, "window", 4096, NULL, 1, NULL, 1);
}
bool swState;
bool swStatePrev = false;
void loop()
{
    vTaskDelete(NULL);
    // vTaskDelete(NULL);
    // swState = digitalRead(SW0);
    // if (swState != swStatePrev)
    // {
    //     Serial.println(swState ? "ON" : "OFF");
    //     digitalWrite(LED, swState);
    // }
    // swStatePrev = swState;
    // delay(10);
}
void window(void *)
{
    bool windowState[] = {false, false, false, false};
    while (true)
    {
        for (auto i = 0; i < WINDOW_LENGTH; i++)
        {
            auto state = digitalRead(windows[i]) == HIGH;
            if (state != windowState[i])
            {
                Serial.printf("window %d: %s\n", i, state ? "ON" : "OFF");
                WindowMessage message = {i, state};
                xQueueSend(uploaderQueue, &message, 0);

                windowState[i] = state;
            }
        }
        delay(100);
    }
}

// サーボモーターを回すタスク
void servoTask(void *)
{
    auto sw1old = digitalRead(SW1);
    auto sw2old = digitalRead(SW2);
    auto state = true;
    servo.write(SERVO_LOCK);
    while (true)
    {
        if (servoState)
        {
            servo.write(SERVO_UNLOCK);
        }
        else
        {
            servo.write(SERVO_LOCK);
        }
        // servo.write(0);
        // Serial.println("0");
        // delay(2000);
        // servo.write(90);
        // Serial.println("90");
        // delay(2000);
        // servo.write(180);
        // Serial.println("180");
        // delay(2000);
        // servo.write(90);
        // Serial.println("90");
        // delay(2000);
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
        delay(100);
    }
}

void watchDore(void *)
{
    Serial.println("watchDore");
    HTTPClient http;
    bool oldState = false;
    while (true)
    {
        http.begin(URL("dore"));
        Serial.println(URL("dore"));
        auto ret = http.GET();
        http.end();
        if (ret != 200)
        {
            Serial.printf("dore: error %d\n", ret);
            continue;
        }
        auto json = http.getString();
        Serial.print("dore: ");
        Serial.println(json);
        auto state = json == "true";
        if (state)
        {
            servoState = true;
        }
        else
        {
            servoState = false;
        }
        if (oldState != state)
        {
            Serial.println(state ? "ON" : "OFF");
            oldState = state;
        }
        oldState = state;
        delay(1);
    }
}

void uploader(void *)
{
    HTTPClient http;
    uint8_t windowState[WINDOW_LENGTH];
    while (true)
    {
        WindowMessage message;
        auto res = xQueueReceive(uploaderQueue, &message, portMAX_DELAY);
        if (res == pdFAIL)
        {
            delay(10);
            continue;
        }

        auto url_tmp = URL("window/%d");
        Serial.println(url_tmp);
        char url[100];

        sprintf(url, url_tmp, message.first + 1);
        http.begin(url);
        auto data = message.second ? "true" : "false";
        Serial.println(url);
        // Serial.printf("up: %s %s\n", url, data);
        disableCore0WDT();
        auto ret = http.PUT(data);
        enableCore0WDT();
        http.end();
        if (ret != 200)
        {
            Serial.printf("error %d\n", ret);
            continue;
        }
        delay(10);
    }
}
