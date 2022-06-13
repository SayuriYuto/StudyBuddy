
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2022 mobizt
 *
 */

// This example shows how to create a document in a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

// #if defined(ESP32)
// #include <WiFi.h>
// #elif defined(ESP8266)
#include <ESP8266WiFi.h>
// #endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "ChotaBheem"
#define WIFI_PASSWORD "kandalassan"

/* 2. Define the API Key */
#define API_KEY "AIzaSyDvpW0XKI2CF5fgdeGaltkZ9nOtTtSIHhs"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "studybuddytimer"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "amey.dhuri21@vit.edu"
#define USER_PASSWORD "qwerty12345"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;
unsigned long sendDataPrevMillis = 0;

#define switchPin D5
int prevState = LOW;
#define LED_BUILTIN 2

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

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
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
    pinMode(switchPin, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
        FirebaseJson content;
        //        Switch is pressed
        // Serial.println(digitalRead(switchPin));
        if (digitalRead(switchPin) == LOW && prevState == HIGH)
        {
            Serial.println("Button was pressed");
            sendDataPrevMillis = millis();
            Serial.println(millis());
            digitalWrite(LED_BUILTIN, LOW);
            prevState = LOW;
        }

    //    Button unpressed
    if (digitalRead(switchPin) == HIGH && prevState == LOW)
    {
        Serial.println("Button is unpressed");
        Serial.println(sendDataPrevMillis);
        Serial.println(millis() - sendDataPrevMillis);
        if ((millis() - sendDataPrevMillis > 15000)&&Firebase.ready()&&sendDataPrevMillis!=0)
        {
            String documentPath = "History/" + String(millis());
            content.set("fields/time/integerValue", String((millis() - sendDataPrevMillis)/1000));
            
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
    }
}
