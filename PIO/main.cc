#include <Arduino.h>
#include <Stream.h>
#include <assert.h>
#include "soc/rmt_struct.h"
#include "esp32-hal.h"
#include "esp_intr.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "freertos/semphr.h"

#include <TDSugar.h>
#include <TDESP32Pins.h>
#include <TDStreamFactory.h>
#include <TDComponent.h>
#include <TDSensor.h>
#include <TDTimer.h>
#include <TDTools.h>
#include <TDSPISlave.h>
#include <TDBusManager.h>
#include <TDI2CSwitch.h>
#include <TDHDRV2667.h>
#include <TDHDRV2605.h>
#include "SDUtilities.h"
#include "WiFiManager.h"
#include "AudioManager.h"
#include <TDCAP1188.h>

#define solenoid_1 12
#define solenoid_2 27
#define solenoid_3 33

#define pressureSensor_1 A3
#define pressureSensor_2 A4

char PV1 = 0;
char PV2 = 0;

unsigned long startTime;
unsigned long endTime;
int incomingChar;

TDHDRV2667 *hDRV2667;

WiFiManager *_WiFiManager;
AudioManager *_AudioManager;

TDCAP1188 * _Cap1188;

Stream *outputStream;

SPIClass _SPI(VSPI);

bool notificationEnabled = !true;
unsigned long notificationTimer = 0;

void WiFiTask(void *args);

class GeneralCommandHandler: public WiFiRemoteCommandHandler
{
  public:
    bool onCommand(String command)
    {
        if(strncmp(command.c_str(), "SE1", 3) == 0)
        {
            digitalWrite(solenoid_1, HIGH);
            if(ACTUATORS_DEBUG) Serial.println("Solenoid 1 On");
            return true;
        }
        else if(strncmp(command.c_str(), "SD1", 3) == 0)
        {
            digitalWrite(solenoid_1, LOW);
            if(ACTUATORS_DEBUG) Serial.println("Solenoid 1 Off");
            return true;
        }
        else if(strncmp(command.c_str(), "SE2", 3) == 0)
        {
            digitalWrite(solenoid_2, HIGH);
            if(ACTUATORS_DEBUG) Serial.println("Solenoid 2 On");
            return true;
        }
        else if(strncmp(command.c_str(), "SD2", 3) == 0)
        {
            digitalWrite(solenoid_2, LOW);
            if(ACTUATORS_DEBUG) Serial.println("Solenoid 2 Off");
            return true;
        }
        else if(strncmp(command.c_str(), "SE3", 3) == 0)
        {
            digitalWrite(solenoid_3, HIGH);
            if(ACTUATORS_DEBUG) Serial.println("Solenoid 3 On");
            return true;
        }
        else if(strncmp(command.c_str(), "SD3", 3) == 0)
        {
            digitalWrite(solenoid_3, LOW);
            if(ACTUATORS_DEBUG) Serial.println("Solenoid 3 Off");
            return true;
        }
        else if(strncmp(command.c_str(), "NED", 3) == 0) // Notifications enable/disable
        {
            parameter = atoi(&(command.c_str()[3]));
            notificationEnabled = (parameter == 1);
            if(ACTUATORS_DEBUG){ Serial.print("Notifications enabled : ");Serial.println(notificationEnabled); }
            return true;
        }

        return false;
    };

    private:
    byte parameter;
};

void setup()
{
    startTime = millis();

    pinMode(led, OUTPUT);
    pinMode(solenoid_1, OUTPUT);
    pinMode(solenoid_2, OUTPUT);
    pinMode(solenoid_3, OUTPUT);
    pinMode(pressureSensor_1, INPUT);
    pinMode(pressureSensor_2, INPUT);

    outputStream = TDStreamFactory::getHWSerialStream(MONITOR_SPEED);
    Serial.println("");
    Serial.println("Starting...");

    TDBusManager::initSPI(&_SPI);
	
	SDUtilities::init();
    
    TDBusManager::initI2C(
        I2C_NUM_0, 
        TD_ESP32_I2C_SDA_PIN, 
        TD_ESP32_I2C_SCL_PIN, 
        ESP32_I2C_FREQUENCY, 
        ESP32_I2C_RX_BUFFER_SIZE, 
        ESP32_I2C_TX_BUFFER_SIZE
    );
    
    ///////////////////////////////////////////////////////////////////////////////////

    //btStop(); // Disable BLE
    _WiFiManager = new WiFiManager("WiFi manager for wireless multi-device support", "WiFi Manager");

    ///////////////////////////////////////////////////////////////////////////////////

    _AudioManager = new AudioManager("Audio manager with I2S support", "Audio Manager", _WiFiManager);
    short mode = AUDIO_MANAGER_MODE_TWO_PORTS_EXTERNAL_I2S;
    _AudioManager->init(mode);

    ///////////////////////////////////////////////////////////////////////////////////

    hDRV2667 = new TDHDRV2667("Haptic Piezo Driver DRV2667", "Haptic DRV2667", _WiFiManager);    
    hDRV2667->init();

    ///////////////////////////////////////////////////////////////////////////////////

    _Cap1188 = new TDCAP1188("Capacitive Sensor CAP1188", "CAP1188", _WiFiManager);
    _Cap1188->init();
    
    ///////////////////////////////////////////////////////////////////////////////////

    _WiFiManager->addRemoteCommandHandler(new GeneralCommandHandler());
    _WiFiManager->addRemoteCommandHandler(new DRV2667CommandHandler(hDRV2667));
    _WiFiManager->addRemoteCommandHandler(new AudioCommandHandler(_AudioManager));

    ///////////////////////////////////////////////////////////////////////////////////

    TaskHandle_t WiFiTaskHandle;
    xTaskCreate(WiFiTask,            // function to run
                "WiFi Task",         // human readable desc
                10 * 1024,           // reserved stack size ///
                NULL,                // parameters
                2,                   // uxPriority
                &WiFiTaskHandle);    // task handle

    ///////////////////////////////////////////////////////////////////////////////////

    endTime = millis();
    LogPrint(F("Loading time (ms): "));
    LogPrintln((int)(endTime - startTime));

    LogPrintln(F("HapBand is ready!"));
    ShowLedSequenceD();

    hDRV2667->play(130,((255*660)/1992),100,1);
    delay(150);
    hDRV2667->play(130,((255*660)/1992),100,1);
    delay(300);
    hDRV2667->play(130,((255*660)/1992),100,1);
    delay(300);
    hDRV2667->play(130,((255*510)/1992),100,1);
    delay(100);
    hDRV2667->play(130,((255*660)/1992),100,1);
    delay(300);
    hDRV2667->play(130,((255*770)/1992),100,1);
    delay(550);
    //hDRV2667->play(230,((255*380)/1992),100,1);
    //delay(1000);

    
    /*
    hDRV2667->play(250,128,50,1);
    delay(25);
    hDRV2667->play(250,1166,250,3);
    delay(500);
    */
    
    /*
    hDRV2667->play(230,135,250,1);
    delay(250);
    hDRV2667->play(230,195,250,1);
    delay(150);
    hDRV2667->play(230,235,250,1);
    delay(250);
    hDRV2667->stop();
    */

    digitalWrite(solenoid_1, HIGH);
    delay(100);
    digitalWrite(solenoid_1, LOW);
    digitalWrite(solenoid_2, HIGH);
    delay(100);
    digitalWrite(solenoid_2, LOW);
    digitalWrite(solenoid_3, HIGH);
    delay(100);
    digitalWrite(solenoid_3, LOW);

    delay(500);

    _AudioManager->playNextA();
    _AudioManager->playNextB();
    delay(500);
    _AudioManager->playNextA();
    _AudioManager->playNextB();
    delay(500);
}

void processSerialInput()
{
    if (outputStream->available())
    {
        incomingChar = outputStream->read();
        
        if (incomingChar == '1')
        {
            //hDRV2667->play(100,400,250,2);
            hDRV2667->play(230,235,250,1);
        }
        else if (incomingChar == '2')
        {
            hDRV2667->stop();
        }
        else if (incomingChar == 'i')
        {
            digitalWrite(solenoid_1, HIGH);
            delay(100);
            digitalWrite(solenoid_1, LOW);
            Serial.println("Solenoid1 On/Off");
        }
        else if (incomingChar == 'o')
        {
            digitalWrite(solenoid_2, HIGH);
            delay(100);
            digitalWrite(solenoid_2, LOW);
            Serial.println("Solenoid2 On/Off");
        }
        else if (incomingChar == 'p')
        {
            digitalWrite(solenoid_3, HIGH);
            delay(100);
            digitalWrite(solenoid_3, LOW);
            Serial.println("Solenoid3 On/Off");
        }
        else if (incomingChar == 'a')
        {
           _AudioManager->playNextA();
           _AudioManager->playNextB();
        }
        else if (incomingChar == 's')
        {
            _AudioManager->playA(4, 5, false);
            _AudioManager->playB(4, 5, false);
        }
        else if (incomingChar == 'd')
        {
            _AudioManager->playA(4, 30, false);
            _AudioManager->playB(4, 30, false);
        }
        else if (incomingChar == 'f')
        {
            _AudioManager->playA(4, 70, false);
            _AudioManager->playB(4, 70, false);
        }
        else if (incomingChar == 'g')
        {
            _AudioManager->playA(4, 100, false);
            _AudioManager->playB(4, 100, false);
        }
        else if (incomingChar == 'h')
        {
            _AudioManager->stopA();
            _AudioManager->stopB();
        }
    }
}

void sendNotifications()
{
    if (_WiFiManager->isClientConnected())
    {
        if(notificationEnabled && (millis() - notificationTimer) > 20) // ms >>> 40 = 25FPS
        {
            notificationTimer = millis();
            
            PV1 = analogRead(pressureSensor_1); 
            PV2 = analogRead(pressureSensor_2); 
            // map(analogRead(pressureSensor1), 0, 4000, 0, 255);
            //if(PV1 >=  1 | PV2 >= 1 ) Serial.println("P1 = " + String((int)PV1) + " P2 = " + String((int)PV2));

            _WiFiManager->sendData("N" + String((int)PV1) + "," + String((int)PV2) + "," + String((int)_Cap1188->getStatus()) + "#");
        }
    }   
}

void WiFiTask(void *args)
{
    while (TRUE)
    {
        _WiFiManager->loop();    

        delay(1); // DO NOT DELETE!!!!!
    }
}

void loop()
{
    //vTaskDelete(null);
    
    //if (!_WiFiManager->isClientConnected()) _AudioManager->loop();
    _AudioManager->loop();

    _Cap1188->loop();

    sendNotifications(); // pressure and capacitive sensors together

    processSerialInput();

    delay(1); // DO NOT DELETE!!!!!
}
