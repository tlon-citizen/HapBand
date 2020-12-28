
#include <EMSChannel.h>
#include <EMSManager.h>

EMSChannel::EMSChannel(byte index, byte pinONOFF, byte pinUD, byte pinINC, byte pinCS, byte pinWiper)
{
    this->index = index;
    this->pinONOFF = pinONOFF;
    this->pinUD = pinUD;
    this->pinINC = pinINC;
    this->pinCS = pinCS;
    this->pinWiper = pinWiper;
}

void EMSChannel::Init()
{
    this->enabled = false;
    this->stopTime = 0;
    this->intensityTMP = NO_INTENSITY_TMP;
	
    Serial.print("\nInitializing Digital POT "); Serial.print((int)index); Serial.println(" ...");
    pinMode(pinONOFF, OUTPUT);
    
    pinMode(pinCS, OUTPUT);
    digitalWrite(pinCS, HIGH); //deselect the POT
    pinMode(pinUD, OUTPUT); 
    pinMode(pinINC, OUTPUT);
    pinMode(pinWiper, INPUT);
    digitalWrite(pinWiper, LOW); // Set analog pin to known state just to be thorough
    digitalWrite(pinCS, HIGH);     //deselect the POT
    Serial.println("\nDigital POT 1 (X9C10XP) started!\n");

    CalibratePOT();

    this->lastTAP = 0;
    _SetIntensity(STARTING_PERCENTAGE);

    Serial.print("\nDigital POT "); Serial.print((int)index); Serial.println(" ready!");
}

bool EMSChannel::IsEnabled()
{
    return this->enabled;
}

void EMSChannel::On()
{
    if(!enabled)
    {
        digitalWrite(pinONOFF, HIGH);
        
        CheckTMPIntensity();
        
        this->enabled = true;
    }
}

void EMSChannel::Off()
{
    if(enabled)
    {
        intensityTMP = _GetIntensity();
        _SetIntensity(0);

        digitalWrite(pinONOFF, LOW);

        this->enabled = false;
    }
}

void EMSChannel::IncreaseIntensity(short value) 
{
    if(!enabled)
    { 
        if(EMS_DEBUG){ Serial.print("EMS CH"); Serial.print((int)index); Serial.println(" must be enabled to change intensity!"); } 
        return;
    }

    CheckTMPIntensity();

    _SetIntensity(_GetIntensity() + value);
}

void EMSChannel::SetPeakDurationAndIntensity(int duration, byte intensity)
{
    if(enabled || stopTime)
    { 
        if(EMS_DEBUG){ Serial.print("EMS CH"); Serial.print((int)index); Serial.println(" is BUSY!"); } 
        return; 
    }

    if (duration > MAX_SIGNAL_TIME) { duration = MAX_SIGNAL_TIME; }
    if (duration < MIN_SIGNAL_TIME) { duration = MIN_SIGNAL_TIME; }

    On();
    _SetIntensity(intensity);

    stopTime = millis() + duration;
}

void EMSChannel::Loop()
{
    if (stopTime && stopTime <= millis()) 
	{
        Off();
        stopTime = 0;
        PrintStatus();
	}
}

void EMSChannel::CheckTMPIntensity()
{
    if(intensityTMP != NO_INTENSITY_TMP)
    {
        _SetIntensity(intensityTMP);
        intensityTMP = NO_INTENSITY_TMP;
    }
}

byte EMSChannel::_GetIntensity()
{
    return (100 - currentTAP);
}

void EMSChannel::_SetIntensity(byte percentage) 
{
    if(percentage < 1) percentage = 1; 
    else if(percentage > 100) percentage = 100;

    currentTAP = 100 - percentage;
    
    if(lastTAP != currentTAP)
    {
        digitalWrite(pinUD, (currentTAP>lastTAP));

        for(int i=0; i < abs(lastTAP-currentTAP); i++)
        {
            SetPOT(TAP_DELAY);
        }

        lastTAP = currentTAP;
    }
}

void EMSChannel::TestTAPCycle()
{
    digitalWrite(pinUD, true);
    for (int tap = 0; tap < NUM_TAPS; tap++)
    {
        SetPOT(TAP_DELAY);
    }
    digitalWrite(pinUD, false);
    for (int tap = 0; tap < NUM_TAPS; tap++)
    {
        SetPOT(TAP_DELAY);
    }
}

String EMSChannel::GetStatusPacket()
{
    //return String((int)index) + (enabled? "1" : "0") + "," + String((int)(100 - currentTAP)) + "," + (stopTime? "T" : "N");
    return String(enabled? "1" : "0") + "," + String((int)(100 - currentTAP)) + "," + (stopTime? "1" : "0");
}

void EMSChannel::PrintStatus()
{
    if(EMS_DEBUG)
    {
        status = 
        "CH" + String((int)index) + " -> " 
        + (enabled? "ON" : "OFF") + " -> " 
        + String((int)(100 - currentTAP)) + " % " 
        + (stopTime? ("Triggered " + String((int)(stopTime-millis())) + "ms") : "");
        
        Serial.println(status.c_str());
    }
}

float EMSChannel::ReadAndPrintADCVoltage()
{
    int sampleADC = analogRead(pinWiper);
    float volts = (sampleADC * ANALOG_REFERENCE) / 1023.0;
    Serial.print("\tADC = ");
    Serial.print(sampleADC);
    Serial.print("\tVoltage = ");
    Serial.print(volts, 1);
    return volts;
}

void EMSChannel::SetPOT(int delayMS) // Perform a wiper movement.
{
    digitalWrite(pinINC, HIGH); // HIGH before falling edge. Not recommended for puksed key to be low when chip select (enable) pulled low.
    delay(delayMS);             // wait for IC/stray capacitance ?
    digitalWrite(pinCS, LOW);   // select the POT
    digitalWrite(pinINC, LOW);  // LOW for effective falling edge
    delay(delayMS);             // wait for IC/stray capacitance ? Tap point copied into non-volatile memory if CS returns HIGH while INC is HIGH
    digitalWrite(pinCS, HIGH);  //deselect the POT
}

void EMSChannel::IterateTAPs(bool directionUP)
{
    digitalWrite(pinUD, directionUP);
    
    Serial.println();
    Serial.print("Wiper Direction ");
    if (directionUP) Serial.println("UP"); else Serial.println("DOWN");
    
    for (int tap = 0; tap < NUM_TAPS; tap++)
    {
        if(EMS_DEBUG)
        {
            String stringOne = "0";
            String stringTwo = "";
            if (directionUP)
            {
                stringOne += String(tap);
                stringTwo = String(tap * STEP_OHMS);
            }
            else
            {
                stringOne += String(NUM_TAPS - tap - 1);
                stringTwo = String((NUM_TAPS - tap - 1) * STEP_OHMS, DEC);
            }
            
            Serial.print("Tap = ");
            Serial.print(stringOne.substring(stringOne.length() - 2));
            float voltage = ReadAndPrintADCVoltage();
            Serial.print("  Ohm = ");
            stringTwo += "     "; //Pad to 1M-1
            Serial.print(stringTwo.substring(0, 6));
            Serial.print("  Law = ");
            Serial.println(float(POT_VALUE) * (voltage / ANALOG_REFERENCE), 0);
        }

        SetPOT(CAL_DELAY); // Move the tap point one count
    }

    Serial.println("");
}

void EMSChannel::CalibratePOT() // on Wiper's full scope
{
    // First pass will ensure the wiper position is a fixed terminal.
    // The second and third iteration will provide an ADC sampling for the increment/decrement away from this fixed terminal.
    
    LogPrintln(F("Calibrating ..."));
    IterateTAPs(false);
    IterateTAPs(true);
    IterateTAPs(false);
}