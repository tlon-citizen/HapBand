#ifndef TDCAP1188_H
#define TDCAP1188_H

#include <TDComponent.h>
#include <TDTools.h>
#include <TDI2CSlave.h>
#include "WiFiManager.h"

#define CAP1188_I2CADDR 0x29

#define CAP1188_SENINPUTSTATUS 0x3
#define CAP1188_MTBLK 0x2A
#define CAP1188_LEDLINK 0x72
#define CAP1188_PRODID 0xFD
#define CAP1188_MANUID 0xFE
#define CAP1188_STANDBYCFG 0x41
#define CAP1188_REV 0xFF
#define CAP1188_MAIN 0x00
#define CAP1188_MAIN_INT 0x01
#define CAP1188_LEDPOL 0x73

class TDCAP1188 : public TDComponent, public TDI2CSlave
{
  public:
    TDCAP1188(const char *argLongDesc, const char *argShortDesc, WiFiManager* _WiFiManager, uint8_t address = CAP1188_I2CADDR);
    virtual void init(uint8_t index = 0);
    uint8_t getStatus();
    void printStatus();
    bool checkPin(uint8_t pin);
    virtual void loop();
    void enableNotifications(bool enable);
    void sendStatus();

  private:

    WiFiManager* _WiFiManager;
    uint8_t capStatus;
    bool notificationEnabled;
    unsigned long notificationTimer;
};

#endif //TDCAP1188_H
