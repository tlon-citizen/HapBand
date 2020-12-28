#ifndef EMS_H
#define EMS_H

#include <TDComponent.h>
#include <TDTools.h>
#include <EMSChannel.h>
#include "WiFiManager.h"

#define NUM_TAPS                100             // X9C10XP specs
#define POT_VALUE               10000L          // Nominal POT value
#define STEP_OHMS               POT_VALUE/99    // Number of ohms per tap point
#define ANALOG_REFERENCE        3.3             // Alter for 3.3V Arduino
#define CAL_DELAY               20              // ms (20)
#define TAP_DELAY               1               // ms (4)
#define STARTING_PERCENTAGE	    100
#define STARTING_PEAK_DURATION  200             // ms
#define STARTING_PEAK_INTENSITY 40              // %
#define MIN_SIGNAL_TIME	        5	            // ms
#define MAX_SIGNAL_TIME	        1000	        // ms
#define NO_INTENSITY_TMP        -1
#define PIN_UD_CH1              12              // --> X9C10XP pin 2
#define PIN_INC_CH1             27              // --> X9C10XP pin 1
#define PIN_CS_CH1	            33              // --> X9C10XP pin 7
#define PIN_WIPER_CH1           A1              // --> X9C10XP pin 5
#define PIN_ON_OFF_CH1          15
#define PIN_UD_CH2              21              // --> X9C10XP pin 2
#define PIN_INC_CH2             4               // --> X9C10XP pin 1
#define PIN_CS_CH2	            26              // --> X9C10XP pin 7
#define PIN_WIPER_CH2           A2              // --> X9C10XP pin 5
#define PIN_ON_OFF_CH2          14

#define PIN_EMG_ANALOG          A3

class EMSManager : public TDComponent
{
  public:
    EMSManager(const char *argLongDesc, const char *argShortDesc, WiFiManager *_WiFiManager);
    void Init();
    void Loop();
    String GetStatusPacket();
    void SendNotifications();
    String GetEMGValue();
    EMSChannel* CH1;
    EMSChannel* CH2;
    bool statusNotificationEnabled;
    bool EMGNotificationEnabled;

  private:
    WiFiManager* _WiFiManager;
    unsigned long statusNotificationTimer;
    unsigned long EMGNotificationTimer;
};

#endif //EMS_H
