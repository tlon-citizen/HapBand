
#include <EMSManager.h>

class EMSCommandHandler : public WiFiRemoteCommandHandler
{
  public:
    EMSCommandHandler(EMSManager* manager)
    {
        commandSource = manager;
    }

    bool onCommand(String command)
    {
        if(strncmp(command.c_str(), "SN", 2) == 0)
        {
            commandSource->statusNotificationEnabled = (atoi(command.substring(2).c_str())==1);
            Serial.print("Status notification: "); Serial.println(commandSource->statusNotificationEnabled);
            return true;
        }
        else if(strncmp(command.c_str(), "EN", 2) == 0)
        {
            commandSource->EMGNotificationEnabled = (atoi(command.substring(2).c_str())==1);
            Serial.print("EMG notification: "); Serial.println(commandSource->EMGNotificationEnabled);
            return true;
        }
        else if(strncmp(command.c_str(), "C1E", 3) == 0)
        {
            commandSource->CH1->On();
            commandSource->CH1->PrintStatus();
            return true;
        }
        else if(strncmp(command.c_str(), "C1D", 3) == 0)
        {
            commandSource->CH1->Off();
            commandSource->CH1->PrintStatus();
            return true;
        }
        else if(strncmp(command.c_str(), "C1I", 3) == 0)
        {
            uint8_t value = atoi(command.substring(3).c_str());
            commandSource->CH1->IncreaseIntensity(value);
            commandSource->CH1->PrintStatus();
            return true;
        }
        else if(strncmp(command.c_str(), "C1P", 3) == 0)
        {
            command = command.substring(3);
            tokens = new char [command.length()+1];
            strcpy(tokens, command.c_str());
            peakDuration = atoi(strtok(tokens, DELIMITERS));
            peakIntensity = atoi(strtok(NULL, DELIMITERS));
            delete tokens;

            commandSource->CH1->SetPeakDurationAndIntensity(peakDuration, peakIntensity);
            commandSource->CH1->PrintStatus();
            return true;
        }
        else if(strncmp(command.c_str(), "C2E", 3) == 0)
        {
            commandSource->CH2->On();
            commandSource->CH2->PrintStatus();
            return true;
        }
        else if(strncmp(command.c_str(), "C2D", 3) == 0)
        {
            commandSource->CH2->Off();
            commandSource->CH2->PrintStatus();
            return true;
        }
        else if(strncmp(command.c_str(), "C2I", 3) == 0)
        {
            uint8_t value = atoi(command.substring(3).c_str());
            commandSource->CH2->IncreaseIntensity(value);
            commandSource->CH2->PrintStatus();
            return true;
        }
        else if(strncmp(command.c_str(), "C2P", 3) == 0)
        {
            command = command.substring(3);
            tokens = new char [command.length()+1];
            strcpy(tokens, command.c_str());
            peakDuration = atoi(strtok(tokens, DELIMITERS));
            peakIntensity = atoi(strtok(NULL, DELIMITERS));
            delete tokens;

            commandSource->CH2->SetPeakDurationAndIntensity(peakDuration, peakIntensity);
            commandSource->CH2->PrintStatus();
            return true;
        }

        return false;
    };

  private:
    EMSManager* commandSource;
    char* tokens;
    uint8_t peakDuration;
    uint8_t peakIntensity;
};

EMSManager::EMSManager(const char *argLongDesc, const char *argShortDesc, WiFiManager* _WiFiManager) : TDComponent(argLongDesc, argShortDesc, TDCOMPONENT_CONNECTION_BUILTIN)
{
    this->_WiFiManager = _WiFiManager;

    CH1 = new EMSChannel(1, PIN_ON_OFF_CH1, PIN_UD_CH1, PIN_INC_CH1, PIN_CS_CH1, PIN_WIPER_CH1);
    CH2 = new EMSChannel(2, PIN_ON_OFF_CH2, PIN_UD_CH2, PIN_INC_CH2, PIN_CS_CH2, PIN_WIPER_CH2);
}

void EMSManager::Init()
{
    pinMode(PIN_EMG_ANALOG, INPUT);

    statusNotificationEnabled = true;
    EMGNotificationEnabled = true;
    statusNotificationTimer = millis();
    EMGNotificationTimer = millis();

    CH1->Init();
    CH2->Init();

    _WiFiManager->addRemoteCommandHandler(new EMSCommandHandler(this));

    Serial.println("EMS manager is ready!");
}

String EMSManager::GetStatusPacket()
{
    return CH1->GetStatusPacket() + "," + CH2->GetStatusPacket();
}

String EMSManager::GetEMGValue()
{
    return String(analogRead(PIN_EMG_ANALOG));
}

void EMSManager::SendNotifications()
{
    if (_WiFiManager->isClientConnected())
    {
        if (statusNotificationEnabled && (millis() - statusNotificationTimer) > 250) // ms >>> 4 FPS
        {
            statusNotificationTimer = millis();
            _WiFiManager->sendData("S" + GetStatusPacket() + "#");
            //if(EMS_DEBUG) Serial.println("S" + GetStatusPacket() + "#");
        }

        if(EMGNotificationEnabled && (millis() - EMGNotificationTimer) > 33) // ms >>> 40 = 25FPS
        {
            EMGNotificationTimer = millis();
            _WiFiManager->sendData("E" + GetEMGValue() + "#");
            //if(EMS_DEBUG) Serial.println("E" + GetEMGValue() + "#");
        }
    }
}

void EMSManager::Loop()
{
    CH1->Loop();
    CH2->Loop();

    //CH1->TestTAPCycle();
    //CH2->TestTAPCycle();
}
