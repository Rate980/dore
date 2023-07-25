QueueHandle_t testQueue;

typedef std::pair<uint8_t, bool> WindowMessage;

#define URL "https://sora-b23da-default-rtdb.firebaseio.com/3601/dore.json"

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

WiFiMulti wifiMulti;

void setup()
{
    wifiMulti.addAP("maruyama", "marufuck");
    Serial.begin(115200);
    testQueue = xQueueCreate(8, sizeof(WindowMessage));
    // xTaskCreatePinnedToCore(testTask, "testTask", 8192, NULL, 1, NULL, 0);
    while (wifiMulti.run() != WL_CONNECTED)
    {
        delay(1);
    }
    Serial.println("connected");
}

void loop()
{
    HTTPClient http;
    http.begin(URL);
    Serial.println("get");
    http.GET();
    Serial.println("end");
    Serial.println(http.getString());
    http.end();
    delay(1);
}

void testTask(void *)
{
    WindowMessage i;
    while (true)
    {
        xQueueReceive(testQueue, &i, portMAX_DELAY);
        Serial.printf("{%d, %s}\n", i.first, i.second ? "true" : "false");
    }
    delay(1);
}
