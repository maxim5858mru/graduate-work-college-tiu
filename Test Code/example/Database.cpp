#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void setup()
{
    Serial.begin(115200);
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    listDir(SD, "/", 0);

    File folder = SD.open("/Database");
    if(!folder || !folder.isDirectory()){         // На случай если папку удалят
        // !!!!! Скачиваем базу данных
    }
    
    // Перебор записей. К сожалению, всю базу не запихнуть в буффер, по этому перебор записей пользователей
    File file = folder.openNextFile();
    while(file)
    {
        DynamicJsonDocument json(600);            // Буффер для файла. Благодаря ему я использую этот тупой костыль-метод
        deserializeJson(json, file);

        // Объявление переменных для файла
        // !!!!! А надо ли???
        uint8_t ID = json["ID"];
        const char* FName = json["First Name"];
        const char* LName = json["Last Name"];
        const char* MName = json["Middle Name"];
        uint8_t Door = json["Door"];

        JsonArray JMethod = json["Method"];
        // 1 - PIN; 2 - FP; 3 - RFID
        bool Methood[3] = {JMethod[0], JMethod[1], JMethod[2]}; 

        const char* PIN = json["PIN"];
        uint8_t FinPrID = json["Fingerprint ID"];

        JsonArray JRFID = json["RFID"];
        byte RFID[10];
        for(int i = 0; i < 10; i++) {RFID[i] = JRFID[i];}

        uint startTime = json["Start Time"];
        uint endTime = json["End Time"];

        // !!!!! Код обработчик
        Serial.println(FName);

        file = folder.openNextFile();
    }
}