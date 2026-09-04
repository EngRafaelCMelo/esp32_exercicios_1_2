constexpr int TEST_LED_PIN = 21;

void setup()
{
    pinMode(TEST_LED_PIN, OUTPUT);
    Serial.begin(115200);
    delay(500);
    Serial.println("DIAGNOSTICO: ESP32 iniciou corretamente");
}

void loop()
{
    static bool level = false;
    level = !level;
    digitalWrite(TEST_LED_PIN, level);
    Serial.printf("DIAGNOSTICO: GPIO21 = %d\n", level ? 1 : 0);
    delay(500);
}
