#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <TDComponent.h>
#include <TDTools.h>
#include <TDESP32Pins.h>
#include "WiFiManager.h"
#include "SDUtilities.h"
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include <vector>

#define AUDIO_MANAGER_MODE_ONE_PORT 1
#define AUDIO_MANAGER_MODE_ONE_PORT_INTERNAL_DAC 2
#define AUDIO_MANAGER_MODE_TWO_PORTS_EXTERNAL_I2S 3

class AudioManager : public TDComponent
{
public:
    AudioManager( const char* argLongDesc, const char* argShortDesc, WiFiManager* _WiFiManager);
    ~AudioManager();
    void init(short mode);
    void loop();

    void playAWithVolume(String buffer, byte preffixLength, bool wait);
    void playNextA();
    void playA(int fileIndex, byte volume, bool wait); // Volume 0..100
    void stopA();
    bool isPlayingA();
    
    void playNextB();
    void playB(int fileIndex, byte volume, bool wait); // Volume 0..100
    void stopB();
    bool isPlayingB();

    void playStream(String buffer, byte preffixLength, byte volumeA, byte volumeB, bool outputA, bool outputB); // Volume 0..100
    void playStreamAOnly(String buffer, byte preffixLength, bool output);

private:
    WiFiManager* _WiFiManager;
    std::vector<std::string> fileNames;
    int currentSoundIndexA;
    int currentSoundIndexB;
    int nSounds;
    
    AudioFileSourceSD* file1 = NULL;
    AudioFileSourceSD *file2 = NULL;
    
    AudioGeneratorWAV* wav1;
    AudioGeneratorWAV *wav2;
    
    AudioOutputI2S* out1;
    AudioOutputI2S *out2;

    bool ready = false;
    int16_t streamPacketSize;

    byte mode;

    std::vector<AudioFileSourceSD*> afs; // audio file sources
};

class AudioCommandHandler: public WiFiRemoteCommandHandler
{
  public:
    AudioCommandHandler(AudioManager* _AudioManager)
    {
      this->_AudioManager = _AudioManager;
    }

    bool onCommand(String command)
    {
        if(strncmp(command.c_str(), "SAA", 3) == 0)
        {
            _AudioManager->playStream(command, 3, volumeA, volumeB, true, false);
            return true;
        }
        if(strncmp(command.c_str(), "SAB", 3) == 0)
        {
            _AudioManager->playStream(command, 3, volumeA, volumeB, false, true);
            return true;
        }
        if(strncmp(command.c_str(), "SBA", 3) == 0) // Both outputs B and A
        {
            _AudioManager->playStream(command, 3, volumeA, volumeB, true, true);
            return true;
        }
        
        else if(strncmp(command.c_str(), "AAW", 3) == 0)
        {
            waitA = atoi(&(command.c_str()[3])) == 1;
            return true;
        }
        else if(strncmp(command.c_str(), "ABW", 3) == 0)
        {
            waitB = atoi(&(command.c_str()[3])) == 1;
            return true;
        }
        else if(strncmp(command.c_str(), "AAV", 3) == 0)
        {
            volumeA = atoi(&(command.c_str()[3]));
            return true;
        }
        else if(strncmp(command.c_str(), "ABV", 3) == 0)
        {
            volumeB = atoi(&(command.c_str()[3]));
            return true;
        }
        else if(strncmp(command.c_str(), "AVP", 3) == 0)
        {
            _AudioManager->playAWithVolume(command, 3, waitA);
            return true;
        }
        else if(strncmp(command.c_str(), "AAP", 3) == 0)
        {
            fileIndex = atoi(&(command.c_str()[3]));
            _AudioManager->playA(fileIndex, volumeA, waitA);
            return true;
        }
        else if(strncmp(command.c_str(), "ABP", 3) == 0)
        {
            fileIndex = atoi(&(command.c_str()[3]));
            _AudioManager->playB(fileIndex, volumeB, waitB);
            return true;
        }
        else if(strncmp(command.c_str(), "PAB", 3) == 0)
        {
            fileIndex = atoi(&(command.c_str()[3]));
            _AudioManager->playA(fileIndex, volumeA, waitA);
            _AudioManager->playB(fileIndex, volumeB, waitB);
            return true;
        }
        else if(strncmp(command.c_str(), "AAS", 3) == 0)
        {
            _AudioManager->stopA();
            return true;
        }
        else if(strncmp(command.c_str(), "ABS", 3) == 0)
        {
            _AudioManager->stopB();
            return true;
        }

        return false;
    }

  private:
    AudioManager* _AudioManager;
    byte fileIndex = 0;
    bool waitA = true;
    byte volumeA = 100;
    bool waitB = true;
    byte volumeB = 100;
};

#endif //AUDIO_MANAGER_H
