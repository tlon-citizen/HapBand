
#include <TDHDRV2667.h>

TDHDRV2667::TDHDRV2667 ( const char *argLongDesc, const char* argShortDesc, WiFiManager* _WiFiManager, TDI2CSwitch* argI2CSwitch, uint8_t address ) :
              TDComponent ( argLongDesc, argShortDesc, TDCOMPONENT_CONNECTION_BUILTIN ), TDI2CSlave ( address )
{
    this->_WiFiManager = _WiFiManager;
    i2CSwitch = argI2CSwitch;
}

TDHDRV2667::TDHDRV2667 ( const char *argLongDesc, const char* argShortDesc, WiFiManager* _WiFiManager, uint8_t address ) :
              TDComponent ( argLongDesc, argShortDesc, TDCOMPONENT_CONNECTION_BUILTIN ), TDI2CSlave ( address )
{
    this->_WiFiManager = _WiFiManager;
}

void TDHDRV2667::init(uint8_t index)
{
    if(index > 0) i2CSwitch->on(index);
  	    i2cWriteRegister8Data8(0x02, 0x00); //Take device out of standby mode
        LogPrint(F("DRV2667 "));
        if(index > 0) 
        {
            LogPrint(F("with index "));
            LogPrint(index);
        }
        LogPrintln(F(" started!"));
	if(index > 0) i2CSwitch->off(index);
}

// https://github.com/yurikleb/DRV2667
// WaveForm Array: [Amplitude, Freq, Cycles, Envelope]
// Amplitude    --  min:0=50v max: 255=100v
// Frequency    --  (0-255) >>> (7.8125-1992.1875)Hz
// Duration     --  Cycles 0-255
// Envelope     --  Ramp up + down
void TDHDRV2667::play(uint8_t amplitude, uint8_t frequency, uint8_t cycles, uint8_t envelope, uint8_t index)
{
    if(DRV2667Debug)
    {
        LogPrint(F("Piezo >> "));
        if(index > 0) LogPrint(F(" Index: "));     LogPrint(index);
        LogPrint(F(" Amplitude: ")); LogPrint(amplitude);
        LogPrint(F(" Frequency: ")); LogPrint((float)(frequency*7.8125)); LogPrint(F("Hz"));
        LogPrint(F(" Cycles: "));    LogPrint(cycles);
        LogPrint(F(" Envelope: "));  LogPrintln(envelope);
    }

    if(index > 0) i2CSwitch->on(index);
        //control
        i2cWriteRegister8Data8(0x02, 0x00); //Take device out of standby mode
        i2cWriteRegister8Data8(0x01, 0x03); //Set Gain 0-3 (0x00-0x03 25v-100v)
        i2cWriteRegister8Data8(0x03, 0x01); //Set sequencer to play WaveForm ID #1
        i2cWriteRegister8Data8(0x04, 0x00); //End of sequence
        //header
        i2cWriteRegister8Data8(0xFF, 0x01); //Set memory to page 1
        i2cWriteRegister8Data8(0x00, 0x05); //Header size -1
        i2cWriteRegister8Data8(0x01, 0x80); //Start address upper uint8_t (page), also indicates Mode 3
        i2cWriteRegister8Data8(0x02, 0x06); //Start address lower uint8_t (in page address)
        i2cWriteRegister8Data8(0x03, 0x00); //Stop address upper uint8_t
        i2cWriteRegister8Data8(0x04, 0x06+3); //Stop address Lower uint8_t // 3 = sizeof(WaveForm)-1
        i2cWriteRegister8Data8(0x05, 0x01); //Repeat count, play WaveForm once
        //WaveForm Data From the array
        i2cWriteRegister8Data8(0x06+0, amplitude);
        i2cWriteRegister8Data8(0x06+1, frequency);
        i2cWriteRegister8Data8(0x06+2, cycles);
        i2cWriteRegister8Data8(0x06+3, envelope);
        //Control
        i2cWriteRegister8Data8(0xFF, 0x00); //Set page register to control space
        i2cWriteRegister8Data8(0x02, 0x01); //Set GO bit (execute WaveForm sequence)
    if(index > 0) i2CSwitch->off(index);
}

void TDHDRV2667::stop(uint8_t index)
{
    if(index > 0) i2CSwitch->on(index);
	      i2cWriteRegister8Data8(0x02, 0x00); //Take device out of standby mode
    if(index > 0) i2CSwitch->off(index);

    if(DRV2667Debug) LogPrintln(F("Piezo >> Stop"));
}

void TDHDRV2667::setAnalogInput(uint8_t index)
{
	if(index > 0) i2CSwitch->on(index);
	    i2cWriteRegister8Data8(0x02, 0x00); //Take device out of standby mode
        i2cWriteRegister8Data8(0x01, 0x07); //Set to analog input + Gain 0-3 (0x04-0x07 25v-100v)
        i2cWriteRegister8Data8(0x02, 0x02); //Set EN_OVERRIDE bit = boost and amplifier active
	if(index > 0) i2CSwitch->off(index);
}

void TDHDRV2667::loop()
{
    /*
    i2CSwitch->on(index);
        if(DRV2667Debug)
        {
          LogPrintln(F(""));
        }
    i2CSwitch->off(index);
    */
}

void TDHDRV2667::parseArray(String buffer, byte preffixLength)
{
    String s = buffer.substring(preffixLength);
    char* ptr = NULL;
    char * b = new char [s.length()+1];
    strcpy(b, s.c_str());
    ptr = strtok(b, ",");  // list of delimiters
    piezoIndex = atoi(ptr); ptr = strtok(NULL, ",");
    amplitude = atoi(ptr); ptr = strtok(NULL, ",");
    frequency = atoi(ptr); ptr = strtok(NULL, ",");
    cycles    = atoi(ptr); ptr = strtok(NULL, ",");
    envelope  = atoi(ptr);
    play(amplitude, frequency, cycles, envelope, piezoIndex);
}
