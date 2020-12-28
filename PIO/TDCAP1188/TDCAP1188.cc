
#include <TDCAP1188.h>

TDCAP1188::TDCAP1188( const char *argLongDesc, const char* argShortDesc, WiFiManager* _WiFiManager, uint8_t address ) :
              TDComponent ( argLongDesc, argShortDesc, TDCOMPONENT_CONNECTION_BUILTIN ), TDI2CSlave ( address )
{
    this->_WiFiManager = _WiFiManager;
    this->capStatus = 0;
    this->notificationEnabled = true;
    this->notificationTimer = 0;
}

void TDCAP1188::init(uint8_t index)
{
    i2cReadRegister8Data8(CAP1188_PRODID);
  
    // LogPrint(F("Product ID: "));
    // LogPrintln(i2cReadRegister8Data8(CAP1188_PRODID));
    // LogPrint(F("Manufacturer ID: "));
    // LogPrintln(i2cReadRegister8Data8(CAP1188_MANUID));
    // LogPrint(F("Revision: "));
    // LogPrintln(i2cReadRegister8Data8(CAP1188_REV));

    if( 
        (i2cReadRegister8Data8(CAP1188_PRODID) != 0x50) ||
        (i2cReadRegister8Data8(CAP1188_MANUID) != 0x5D) ||
        (i2cReadRegister8Data8(CAP1188_REV)    != 0x83)
    )
    {
        LogError(F("CAP1188 not found!"));
		while (1);
    }
  
    // allow multiple touches
    i2cWriteRegister8Data8(CAP1188_MTBLK, 0); 
    // Have LEDs follow touches
    i2cWriteRegister8Data8(CAP1188_LEDLINK, 0xFF);
    // speed up a bit
    i2cWriteRegister8Data8(CAP1188_STANDBYCFG, 0x30);
    
    LogPrintln(F("CAP1188 Started!"));
}

uint8_t TDCAP1188::getStatus()
{
  return capStatus;
}

void TDCAP1188::printStatus()
{
    if (capStatus != 0)
    {
        for (uint8_t i=0; i<8; i++)
        {
            if (capStatus & (1 << i))
            {
                LogPrint(F("C")); LogPrint(i+1); LogPrint(F("\t"));
            }
        }
        
        LogPrintln(F(""));
    }
}

bool TDCAP1188::checkPin(uint8_t pin)
{
	if (capStatus == 0) return false;
	return (capStatus & (1 << pin));
}

void TDCAP1188::loop()
{
    capStatus = i2cReadRegister8Data8(CAP1188_SENINPUTSTATUS);
    if (capStatus) 
    {
        i2cWriteRegister8Data8(CAP1188_MAIN, i2cReadRegister8Data8(CAP1188_MAIN) & ~CAP1188_MAIN_INT);
    }
}

void TDCAP1188::sendStatus()
{
    if(WIFI_MANAGER_DEBUG) printStatus();

    if (_WiFiManager->isClientConnected())
    {
        if(notificationEnabled && (millis() - notificationTimer) > 20) // ms >>> 40 = 25FPS
        {
            notificationTimer = millis();

            _WiFiManager->sendData("C" + String((int)capStatus) + "#");
        }
    }   
}
