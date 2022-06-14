#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

#define WIFI_SSID "ChotaBheem"
#define WIFI_PASSWORD "kandalassan"

#define API_KEY "AIzaSyD2Wp0dXTHWnKZzlaoXS1r-hwyZVNw9plk"
#define FIREBASE_PROJECT_ID "studybuddy-28b79"
#define USER_EMAIL "amey.dhuri21@vit.edu"
#define USER_PASSWORD "qwerty12345"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long dataMillis = 0;
int count = 0;
unsigned long sendDataPrevMillis = 0;
unsigned long lastFBcheck = 0;
bool timermode = LOW;
String currentDate;

#define switchPin D5
int prevState = LOW;
#define LED_BUILTIN 2

#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void studybuddytext();
void wifibar(bool flag);
void firebaseConnected(bool flag);
void showclock(char timec[], bool flag); 
void studying(bool flag, char timec[]);

void setup()
{

    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.display();
    studybuddytext();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        wifibar(LOW);
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    // For sending payload callback
    // config.cfs.upload_callback = fcsUploadCallback;
    pinMode(switchPin, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    timeClient.begin();
    timeClient.setTimeOffset(19800);
}

void loop()
{
    if (millis() - lastFBcheck > 1000)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            wifibar(LOW);
            firebaseConnected(LOW);
        }
        else
        {
            wifibar(HIGH);
            firebaseConnected(HIGH);
        }
        if (timermode == HIGH)
        {
            char pp[5];
            //      int hourspassed = (sendDataPrevMillis)/3600000;
            int minutespassed = (millis() - sendDataPrevMillis) / 60000;
            int secondspassed = ((millis() - sendDataPrevMillis)-(minutespassed*60000)) / 1000;
            sprintf(pp, "%02d:%02d", minutespassed, secondspassed);
//            Serial.println(tt);
            showclock(pp, HIGH);
            
            timeClient.update();
            time_t epochTime = timeClient.getEpochTime();
            struct tm *ptm = gmtime ((time_t *)&epochTime);
            int monthDay = ptm->tm_mday;
            int currentMonth = ptm->tm_mon+1;
            int currentYear = ptm->tm_year+1900;
            currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
            
            char tt[5];
            sprintf(tt, "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
            studying(HIGH, tt);
        }
        else
        {
            timeClient.update();
            char tt[5];
            sprintf(tt, "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
            //      Serial.println(tt);
            showclock(tt, HIGH);
            studying(LOW, tt);
        }
        lastFBcheck = millis();
    }

    FirebaseJson content;
    if (digitalRead(switchPin) == LOW && prevState == HIGH)
    {
        Serial.println("Button was pressed");
//        studying(HIGH);
        sendDataPrevMillis = millis();
        Serial.println(millis());
        digitalWrite(LED_BUILTIN, LOW);
        timermode = HIGH;
        prevState = LOW;
    }

    //    Button unpressed
    if (digitalRead(switchPin) == HIGH && prevState == LOW)
    {
        Serial.println("Button is unpressed");
        Serial.println(sendDataPrevMillis);
        Serial.println(millis() - sendDataPrevMillis);
        if ((millis() - sendDataPrevMillis > 15000) && Firebase.ready() && sendDataPrevMillis != 0)
        {
            String documentPath = "History/" + String(millis());
            content.set("fields/time/integerValue", String((millis() - sendDataPrevMillis) / 1000));
            content.set("fields/date/stringValue", String(currentDate));


            String doc_path = "projects/";
            doc_path += FIREBASE_PROJECT_ID;
            doc_path += "/databases/(default)/documents/coll_id/doc_id";

            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            else
                Serial.println(fbdo.errorReason());
            Serial.println("Set bool... OK Button unpressed");
            digitalWrite(LED_BUILTIN, HIGH);
        }
        prevState = HIGH;
        sendDataPrevMillis = 0;
        timermode = LOW;
    }
}

void wifibar(bool flag)
{
    if (flag == HIGH)
    {
        //  Serial.println("Yeswifibar");
        display.fillRect(120, 0, 7, 9, BLACK);
        display.drawRect(126, 1, 1, 8, WHITE);
        display.drawRect(124, 3, 1, 6, WHITE);
        display.drawRect(122, 5, 1, 4, WHITE);
        display.drawRect(120, 7, 1, 2, WHITE);
        display.display();
    }
    else
    {
        display.fillRect(120, 0, 7, 9, BLACK);
        display.drawRect(124, 1, 1, 6, WHITE);
        display.drawPixel(124, 8, WHITE);
        display.display();
    }
}

void firebaseConnected(bool flag)
{
    if (flag == HIGH)
    {
        //  Serial.println("Firebase connected");
        display.fillRect(108, 1, 11, 8, BLACK);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(108, 2);
        display.println("FB");
        display.display();
    }
    else
    {
        display.fillRect(108, 1, 11, 8, BLACK);
    }
}

void studybuddytext()
{
    display.fillRect(0, 0, 58, 7, BLACK);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(1, 1);
    display.println("STUDY BUDDY");
    display.display();
}

void showclock(char timec[], bool flag) //flag useless
{
    if (flag == HIGH)
    {
        display.fillRect(7, 20, 125, 28, BLACK);
        display.setTextColor(WHITE);
        display.setTextSize(4);
        display.setCursor(7, 20);
        display.println(timec);
        display.display();
    }
    else
    {
        display.fillRect(7, 20, 125, 28, BLACK);
        display.display();
    }
}

void studying(bool flag, char times[])
{
    if (flag == LOW)
    {
        display.fillRect(0, 56, 128, 5, BLACK);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(24, 56);
        display.println("Start Focusing!!");
        display.display();
    }
    else
    {
        display.fillRect(0, 56, 128, 8, BLACK);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(51, 56);
        display.println(times);
        display.display();
    }
}

// The Firestore payload upload callback function
void fcsUploadCallback(CFS_UploadStatusInfo info)
{
    if (info.status == fb_esp_cfs_upload_status_init)
    {
        Serial.printf("\nUploading data (%d)...\n", info.size);
    }
    else if (info.status == fb_esp_cfs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_cfs_upload_status_complete)
    {
        Serial.println("Upload completed ");
    }
    else if (info.status == fb_esp_cfs_upload_status_process_response)
    {
        Serial.print("Processing the response... ");
    }
    else if (info.status == fb_esp_cfs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}
