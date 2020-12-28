#ifndef EMS_CHANNEL_H
#define EMS_CHANNEL_H

#include <TDTools.h>

class EMSChannel
{
  public:
    EMSChannel(byte index, byte pinONOFF, byte pinUD, byte pinINC, byte pinCS, byte pinWiper);
    void Init();
    bool IsEnabled();
    void On();
    void Off();
    void IncreaseIntensity(short value); // -100..100
    void SetPeakDurationAndIntensity(int duration, byte intensity); // 0..1000ms
    void Loop();
    void TestTAPCycle();
    String GetStatusPacket();
    void PrintStatus();

  private:
    void CheckTMPIntensity();
    byte _GetIntensity(); // 0..100%
    void _SetIntensity(byte percentage); // 0..100%
    float ReadAndPrintADCVoltage();
    void SetPOT(int delayMS); // Perform a wiper movement.
    void IterateTAPs(bool directionUP);
    void CalibratePOT(); // on Wiper's full scope

    byte index;
    byte pinONOFF;
    byte pinUD;
    byte pinINC;
    byte pinCS;
    byte pinWiper;
    byte currentTAP;
    byte lastTAP;
    int intensityTMP;
    unsigned long stopTime;
    bool enabled;
    String status;
};

#endif //EMS_CHANNEL_H
