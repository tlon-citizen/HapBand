#ifndef TDHDRV2667_H
#define TDHDRV2667_H

#include <TDComponent.h>
#include <TDTools.h>
#include <TDI2CSlave.h>
#include <TDI2CSwitch.h>
#include "WiFiManager.h"

#define DRV2667_ADDR 0x59


class TDHDRV2667 : public TDComponent, public TDI2CSlave
{
  public:
    TDHDRV2667(const char *argLongDesc, const char *argShortDesc, WiFiManager* _WiFiManager, TDI2CSwitch *argI2CSwitch, uint8_t address = DRV2667_ADDR);
    TDHDRV2667(const char *argLongDesc, const char *argShortDesc, WiFiManager* _WiFiManager, uint8_t address = DRV2667_ADDR);
    virtual void init(uint8_t index = 0);
    virtual void play(uint8_t amplitude, uint8_t frequency, uint8_t cycles, uint8_t envelope, uint8_t index = 0);
    virtual void stop(uint8_t index = 0);
    virtual void setAnalogInput(uint8_t = 0);
    virtual void loop();
    virtual void parseArray(String buffer, byte preffixLength);

  private:
    uint8_t piezoIndex;
    uint8_t amplitude;
    uint8_t frequency;
    uint8_t cycles;
    uint8_t envelope;
    TDI2CSwitch *i2CSwitch;
    WiFiManager* _WiFiManager;
};

class DRV2667CommandHandler: public WiFiRemoteCommandHandler
{
  public:
    DRV2667CommandHandler(TDHDRV2667* argDriver)
    {
      driver = argDriver;
    }

    bool onCommand(String command)
    {
        if(strncmp(command.c_str(), "PIE", 3) == 0) // Enable the piezo actuator
        {
            driver->parseArray(command, 3);
            return true;
        }
        else if(strncmp(command.c_str(), "PID", 3) == 0) // Disable the piezo actuator
        {
            uint8_t actuatorIndex = atoi(&(command.c_str()[3]));
            driver->stop(actuatorIndex);
            return true;
        }

        return false;
    }

  private:
    TDHDRV2667* driver;
};

#endif //TDHDRV2667_H
