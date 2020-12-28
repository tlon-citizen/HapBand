using System;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum LRAMode { EffectMode, RealTimeMode }
public enum FeedbackOutput { Piezo=0, LRA=1, Solenoid=2, Audio=3 }

public class HapBandManager : MonoBehaviour
{
    [SerializeField] public DeviceServer deviceServer;

    public Texture2D HandTexture;
    public AudioClip[] HapticAudioClips;

    [HideInInspector] public LRAMode lraMode;
    [HideInInspector] public FeedbackOutput feedbackOutput;

    [HideInInspector] public byte solenoidIndex = 1;
    [HideInInspector] public byte lraIndex = 0;
    [HideInInspector] public byte piezoIndex = 0;
    [HideInInspector] public byte audioIndex = 0;

    [HideInInspector] public byte effectID = 12; // Page 57 on http://www.ti.com/lit/ds/symlink/drv2605.pdf 
    [HideInInspector] public byte realTimeValue = 32;

    [HideInInspector] public int frequency = 1200; // Frequency 7.8125 Hz -- 1992,1875 Hz
    [HideInInspector] public byte amplitud = 255;
    [HideInInspector] public byte cycles = 8;
    [HideInInspector] public byte envelope = 1;
    [HideInInspector] public float durationMS = 0; //ms

    //[HideInInspector] public string batteryLevel = ""; // Batteries stick around 3.7V then slowly sink down to 3.2V. TODO: Display a warning below 3.7V 

    [HideInInspector] public string deviceName;
    [HideInInspector] public HapBand device = null;

    private string stream;
    private byte value;
    private const float DRV2667_MIN_FREQ_BASE = 7.8125f;
    private const float DRV2667_MAX_FREQ_BASE = 255 * DRV2667_MIN_FREQ_BASE;

    public void Start()
    {
        deviceName = FindObjectOfType<HapBand>().deviceName;
        feedbackOutput = FeedbackOutput.Solenoid;
    }

    void Update()
    {
        if (device == null && deviceServer.connectedDevicesByName.ContainsKey(deviceName))
        {
            device = (HapBand)deviceServer.connectedDevicesByName[deviceName];
            device.notifications = false;
        }
    }

    // Commands

    public void setLRAMode(LRAMode mode)
    {
        lraMode = mode;

        if (mode.Equals(LRAMode.EffectMode))
            device.SendCommand("LME" + lraIndex);
        else if (mode.Equals(LRAMode.RealTimeMode))
            device.SendCommand("LMR" + lraIndex);
    }

    public void triggerLRA(bool enabled)
    {
        value = ((lraMode.Equals(LRAMode.EffectMode)) ? effectID : realTimeValue);
        device.SendCommand("LR" + (enabled ? ("E" + value + "," + lraIndex) : ("D" + lraIndex)));
    }

    public void triggerPiezo(bool enabled, String piezoValues)
    {
        device.SendCommand("PI" + (enabled ? ("E" + piezoIndex + "," + piezoValues) : ("D" + piezoIndex)));
    }

    public void triggerPiezo(bool enabled)
    {
        triggerPiezo(enabled, amplitud + "," + (byte)(frequency / DRV2667_MIN_FREQ_BASE) + "," + cycles + "," + envelope);
    }
    
    public void triggerSolenoid(bool enabled)
    {
        device.SendCommand("S" + (enabled ? "E" : "D") + solenoidIndex);
    }

    public void disableSolenoid()
    {
        device.SendCommand("SD" + solenoidIndex);
    }

    public void triggerSolenoid(int ms)
    {
        device.SendCommand("SE" + solenoidIndex);
        Invoke("disableSolenoid", (float)(1000.0f/ms));
    }

    ///////////////////////////////////////////////////////////////////////////////

    public void SetWaitFlagA(bool waitA) // Wait until the current played file or sample is done
    {
        device.SendCommand("AAW" + (waitA? "1" : "0"));
    }

    public void SetWaitFlagB(bool waitB) // Wait until the current played file or sample is done
    {
        device.SendCommand("ABW" + (waitB ? "1" : "0"));
    }

    public void SetVolumeA(byte volumeA) // 0 .. 100
    {
        device.SendCommand("AAV" + volumeA);
    }

    public void SetVolumeB(byte volumeB) // 0 .. 100
    {
        device.SendCommand("ABV" + volumeB);
    }

    public void triggerAudio(bool enabled, bool enabledA, bool enabledB)
    {
        if(enabled)
        {
            if(enabledA && !enabledB)
                device.SendCommand("AAP" + audioIndex);
            if (!enabledA && enabledB)
                device.SendCommand("ABP" + audioIndex);
            if (enabledA && enabledB)
                device.SendCommand("PAB" + audioIndex);
        }
        else
        {
            if (enabledA && !enabledB)
                device.SendCommand("AAS");
            if (!enabledA && enabledB)
                device.SendCommand("ABS");
            if (enabledA && enabledB)
            {
                device.SendCommand("AAS");
                device.SendCommand("ABS");
            }
        }
    }

    public void PlayStream(bool enabledA, bool enabledB)
    {
        AudioClip clip = HapticAudioClips[0];
        int nSamples = clip.samples;

        if (clip.channels > 1)
        {
            Debug.LogError("Trying to stream an invalid sample set!!!");
            return;
        }

        float[] wav = new float[nSamples];  

        clip.GetData(wav, 0);

        int chunkSize = 256;
        chunkSize = Math.Min(nSamples, chunkSize);
        int numChunks = nSamples / chunkSize;
        int lastChunkSize = nSamples % chunkSize;

        for (int c = 0; c < numChunks - 1; c++)
        {
            stream = chunkSize + ","; // size, sample1, sample2, samplen

            for (int i = 0; i < chunkSize; i++)
            {
                stream += (int)(wav[i + (chunkSize*c)] * 32768) + ",";
            }

            if (enabledA && !enabledB)
                device.SendCommand("SAA" + stream);
            if (!enabledA && enabledB)
                device.SendCommand("SAB" + stream);
            if (enabledA && enabledB)
                device.SendCommand("SBA" + stream);
        }

        if(lastChunkSize > 0)
        {
            stream = lastChunkSize + ","; // size, sample1, sample2, samplen

            for (int i = 0; i < lastChunkSize-1; i++)
            {
                stream += (int)(wav[i + (numChunks*chunkSize)] * 32768) + ",";
            }
            stream += (int)(wav[nSamples-1] * 32768);

            if (enabledA && !enabledB)
                device.SendCommand("SAA" + stream);
            if (!enabledA && enabledB)
                device.SendCommand("SAB" + stream);
            if (enabledA && enabledB)
                device.SendCommand("SBA" + stream);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////

    public void toggleNotifications()
    {
        device.ToggleNotifications();
    }

    //public void triggerLRAAndSolenoid(bool enabled)
    //{
    //    value = ((lraMode.Equals(LRAMode.EffectMode)) ? effectID : realTimeValue);
    //    device.SendCommand("ALS" + (enabled ? ("E" + lraIndex + "," + solenoidIndex + "," + value) : ("D" + lraIndex + "," + solenoidIndex)));
    //}

    public void triggerFeedback(bool enabled, bool enabledA, bool enabledB)
    {
        if (!isReady()) return;

        switch (feedbackOutput)
        {
            case FeedbackOutput.LRA:
                triggerLRA(enabled);
                break;
            case FeedbackOutput.Piezo:
                triggerPiezo(enabled);
                break;
            case FeedbackOutput.Solenoid:
                triggerSolenoid(enabled);
                break;
            case FeedbackOutput.Audio:
                triggerAudio(enabled, enabledA, enabledB);
                break;
            //case FeedbackOutput.LRAAndSolenoid:
            //    triggerLRAAndSolenoid(enabled);
            //    break;
            default:
                Debug.LogError("Unrecognized Option");
                break;
        }

        ///Debug.Log(string.Format(sw.ElapsedMilliseconds + "Fingertip feedback >>> {0}", enabled));
    }

    public bool isReady()
    {
        return device != null;
    }

    private void OnGUI()
    {
        if (Input.GetKeyDown(KeyCode.N)) // Notifications for pressure and capacitive sensors
        {
            toggleNotifications();
        }
        else if (Input.GetKeyDown(KeyCode.Y)) // General feedback
        {
            triggerFeedback(true, true, true);
        }
        else if (Input.GetKeyDown(KeyCode.X))
        {
            triggerFeedback(false, true, true);
        }
        else if (Input.GetKeyDown(KeyCode.Q)) // LRA
        {
            //effectID // 1..123
            //setLRAMode(LRAMode.EffectMode);
            //realTimeValue // 0..127
            //setLRAMode(LRAMode.RealTimeMode);
            triggerLRA(true);
        }
        else if (Input.GetKeyDown(KeyCode.W))
        {
            triggerLRA(false);
        }
        else if (Input.GetKeyDown(KeyCode.I)) // Solenoid
        {
            //solenoidIndex
            triggerSolenoid(true);
        }
        else if (Input.GetKeyDown(KeyCode.O))
        {
            //solenoidIndex
            triggerSolenoid(false);
        }
        else if (Input.GetKeyDown(KeyCode.P))
        {
            //solenoidIndex
            triggerSolenoid(200);
        }
        else if (Input.GetKeyDown(KeyCode.A)) // Audio
        {
            //SetWaitFlagA(true);
            //SetWaitFlagB(true);
            //SetVolumeA(50);
            //SetVolumeB(50);

            //audioIndex
            triggerAudio(true, true, true);
        }
        else if (Input.GetKeyDown(KeyCode.S))
        {
            //audioIndex
            triggerAudio(false, true, true);
        }
        else if (Input.GetKeyDown(KeyCode.D))
        {
            //audioIndex
            PlayStream(true, true);
        }
        else if (Input.GetKeyDown(KeyCode.Alpha1)) // Piezo
        {
            triggerPiezo(true, "255,18,7,9");
        }
        else if (Input.GetKeyDown(KeyCode.Alpha2))
        {
            triggerPiezo(true, "255,25,50,9");
        }
        else if (Input.GetKeyDown(KeyCode.Alpha3))
        {
            triggerPiezo(true, "100,288,15,9");
        }
        else if (Input.GetKeyDown(KeyCode.Alpha4))
        {
            triggerPiezo(true, "255,50,5,9");
        }
        else if (Input.GetKeyDown(KeyCode.Alpha5))
        {
            triggerPiezo(true, "255,40,2,3");
        }
        else if (Input.GetKeyDown(KeyCode.Alpha6))
        {
            triggerPiezo(false);
        }
    }
}