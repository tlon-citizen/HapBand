
#include <AudioManager.h>
#include "driver/i2s.h"

AudioManager::AudioManager(const char *argLongDesc, const char *argShortDesc, WiFiManager* _WiFiManager) : TDComponent(argLongDesc, argShortDesc, TDCOMPONENT_CONNECTION_BUILTIN)
{
    this->_WiFiManager = _WiFiManager;
}

AudioManager::~AudioManager()
{
    for(std::vector<AudioFileSourceSD*>::iterator it = afs.begin(); it != afs.end(); ++it) 
    {
        if (*it != NULL)
        {
            (*it)->close();
            delete (*it);
        }
    }
}

void AudioManager::init(short mode)
{
    currentSoundIndexA = 0;
    currentSoundIndexB = 0;

    //////////////////////////////////////////////////////

    //if(SDUtilities::isReady) // TO DO
    {
        fileNames = SDUtilities::listDir(SD, "/wav");
        nSounds = fileNames.size();
        LogPrint(nSounds);
        LogPrintln(F(" audio files found."));

        // Requires a change in SD.h >>> ::begin(...uint8_t max_files=?)
        for (byte i = 0; i < nSounds; i++)
        {
            afs.push_back(new AudioFileSourceSD(fileNames[i].c_str()));
        }
    }

    ///////////////////////////////////////////////////////

    if(mode == AUDIO_MANAGER_MODE_ONE_PORT)
    {
        out1 = new AudioOutputI2S(I2S_NUM_0, AudioOutputI2S::EXTERNAL_I2S);
        out1->SetPinout(TD_ESP32_I2S_0_BCK_PIN, TD_ESP32_I2S_0_WS_PIN, TD_ESP32_I2S_0_DATA_PIN);
        wav1 = new AudioGeneratorWAV();

        out2 = NULL;
        wav2 = NULL;
    }
    else if(mode == AUDIO_MANAGER_MODE_ONE_PORT_INTERNAL_DAC)
    {
        out1 = new AudioOutputI2S(I2S_NUM_0, AudioOutputI2S::INTERNAL_DAC);
        out1->SetPinout(TD_ESP32_I2S_0_BCK_PIN, TD_ESP32_I2S_0_WS_PIN, TD_ESP32_I2S_0_DATA_PIN);
        wav1 = new AudioGeneratorWAV();

        //out 2 can not be initialized; ESP32 only supports internal DAC on port 0
        out2 = NULL;
        wav2 = NULL;
    }
    else if (mode == AUDIO_MANAGER_MODE_TWO_PORTS_EXTERNAL_I2S)
    {
        out1 = new AudioOutputI2S(I2S_NUM_0, AudioOutputI2S::EXTERNAL_I2S);
        out1->SetPinout(TD_ESP32_I2S_0_BCK_PIN, TD_ESP32_I2S_0_WS_PIN, TD_ESP32_I2S_0_DATA_PIN);
        wav1 = new AudioGeneratorWAV();

        out2 = new AudioOutputI2S(I2S_NUM_1, AudioOutputI2S::EXTERNAL_I2S);
        out2->SetPinout(TD_ESP32_I2S_1_BCK_PIN, TD_ESP32_I2S_1_WS_PIN, TD_ESP32_I2S_1_DATA_PIN);
        wav2 = new AudioGeneratorWAV();
    }

    this->mode = mode;

    ///////////////////////////////////////////////////////
    
    ready = true; 

    LogPrintln(F("Audio manager ready!"));
}

void AudioManager::loop()
{
    if (ready)
    {
        if (wav1->isRunning())
        {
            if (!wav1->loop())
            {
                wav1->stop();
            }
        }
        else
        {
            out1->stop();
        }

        if (wav2 != NULL && wav2->isRunning())
        {
            if (!wav2->loop())
            {
                wav2->stop();
            }
        }
        else
        {
            out2->stop();
        }
    }
}

//////////////////////////////////////////////////////

void AudioManager::playNextA()
{
    if (currentSoundIndexA == nSounds) currentSoundIndexA = 0;
    playA(currentSoundIndexA, 100, true);
    currentSoundIndexA++;
}

void AudioManager::playAWithVolume(String buffer, byte preffixLength, bool wait)
{
    String s = buffer.substring(preffixLength);
    char* ptr = NULL;
    char * b = new char [s.length()+1];
    strcpy(b, s.c_str());
    ptr = strtok(b, ",");  // list of delimiters

    byte index = atoi(ptr); ptr = strtok(NULL, ",");
    byte volume = atoi(ptr);
    
    playA(index, volume, wait);
}

void AudioManager::playA(int fileIndex, byte volume, bool wait)
{
    if(!ready) return;
    
    if(wait)
    {
        if(isPlayingA()) return;
    }
    else
    {
        stopA();
    }

    if(ACTUATORS_DEBUG) Serial.printf("HAPTIC REACTOR PORT A STARTED %s \n", fileNames[fileIndex].c_str());

    file1 = afs[fileIndex];
    file1->seek(0, SEEK_SET);
    wav1->begin(file1, out1, volume);
}

bool AudioManager::isPlayingA()
{
    return (ready && wav1->isRunning());
}

void AudioManager::stopA()
{
    if (isPlayingA())
    {
        wav1->stop();
        out1->stop();
    }
}

//////////////////////////////////////////////////////

void AudioManager::playNextB()
{
    if (currentSoundIndexB == nSounds) currentSoundIndexB = 0;
    playB(currentSoundIndexB, 100, true);
    currentSoundIndexB++;
}

void AudioManager::playB(int fileIndex, byte volume, bool wait)
{
    if(!ready) return;
    
    if(wait)
    {
        if(isPlayingB()) return;
    }
    else
    {
        stopB();
    }

    if(ACTUATORS_DEBUG) Serial.printf("HAPTIC REACTOR PORT B STARTED %s \n", fileNames[fileIndex].c_str());

    file2 = afs[fileIndex];
    file2->seek(0, SEEK_SET);

    wav2->begin(file2, out2, volume);
}

bool AudioManager::isPlayingB()
{
    return (ready && wav2->isRunning());
}

void AudioManager::stopB()
{
    if( isPlayingB() )
    {
        wav2->stop();
        out2->stop();
    }
}

/////////////////////////////////////////////////////////////////////////////

void AudioManager::playStream(String buffer, byte preffixLength, byte volumeA, byte volumeB, bool outputA, bool outputB)
{
    if(!ready) return;

    String s = buffer.substring(preffixLength);
    char* ptr = NULL;
    char * b = new char [s.length()+1];
    strcpy(b, s.c_str());
    ptr = strtok(b, ",");  // list of delimiters

    streamPacketSize = atoi(ptr); ptr = strtok(NULL, ",");
    int16_t *samples = new int16_t[streamPacketSize];

    for (size_t i = 0; i < streamPacketSize-1; i++)
    {
        samples[i] = atoi(ptr); ptr = strtok(NULL, ",");
    }

    samples[streamPacketSize-1] = atoi(ptr);
    
    if(outputA) out1->ConsumeSamplesMono(samples, streamPacketSize, volumeA);
    if(outputB) out2->ConsumeSamplesMono(samples, streamPacketSize, volumeB);

    delete samples;
}


