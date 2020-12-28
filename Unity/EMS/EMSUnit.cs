
using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

[Serializable]
public class EMSUnit : AbstractDevice
{
    [ReadOnly] public bool EMGNotifications = true;
    [ReadOnly] public float muscleSensorAnalogValue = 0;

    [ReadOnly] public bool StatusNotifications = true;
    [ReadOnly] public bool CH1Enabled = false;
    [ReadOnly] public byte CH1Intensity = 0;
    [ReadOnly] public bool CH1OnPeak = false;
    [ReadOnly] public bool CH2Enabled = false;
    [ReadOnly] public byte CH2Intensity = 0;
    [ReadOnly] public bool CH2OnPeak = false;
    
    private static System.Diagnostics.Stopwatch sw = System.Diagnostics.Stopwatch.StartNew();

    private long muscleSensorT1 = sw.ElapsedMilliseconds;
    private long muscleSensorT2 = 0;
    private long muscleSensorPacketCounter;
    private long muscleSensorLastPPS;

    private long statusT1 = sw.ElapsedMilliseconds;
    private long statusT2 = 0;
    private long statusPacketCounter;
    private long statusLastPPS;

    public void ToggleStatusNotification()
    {
        StatusNotifications = !StatusNotifications;
        SendCommand("SN" + (StatusNotifications ? "1" : "0"));
    }

    public void ToggleEMGNotification()
    {
        EMGNotifications = !EMGNotifications;
        SendCommand("EN" + (EMGNotifications ? "1" : "0"));
    }

    public override void ProcessPacket()
    {
        parsingToken = currentPacket[0];
        currentPacket = currentPacket.TrimStart(parsingToken);
        packetValues = currentPacket.Split(',');

        if (parsingToken == 'E' && packetValues.Length == 1)
        {
            muscleSensorAnalogValue = float.Parse(packetValues[0]);

            if (deviceDebug)
            {
                muscleSensorT2 = sw.ElapsedMilliseconds;
                muscleSensorPacketCounter++;
                if (muscleSensorT2 - muscleSensorT1 >= 1000)
                {
                    muscleSensorLastPPS = muscleSensorPacketCounter;
                    muscleSensorT1 = muscleSensorT2;
                    muscleSensorPacketCounter = 0;
                }
            }
        }
        else if (parsingToken == 'S' && packetValues.Length == 6)
        {
            try
            {
                CH1Enabled = (Byte.Parse(packetValues[0]) == 1);
                CH1Intensity = Byte.Parse(packetValues[1]);
                CH1OnPeak = (Byte.Parse(packetValues[2]) == 1);
                CH2Enabled = (Byte.Parse(packetValues[3]) == 1);
                CH2Intensity = Byte.Parse(packetValues[4]);
                CH2OnPeak = (Byte.Parse(packetValues[5]) == 1);
            }
            catch (Exception e)
            {
                Debug.Log(e.Message);
            }

            if (deviceDebug)
            {
                statusT2 = sw.ElapsedMilliseconds;
                statusPacketCounter++;
                if (statusT2 - statusT1 >= 1000)
                {
                    statusLastPPS = statusPacketCounter;
                    statusT1 = statusT2;
                    statusPacketCounter = 0;
                }
            }
        }
        else
        {
            Debug.Log(string.Format("Lost Packet >>> {0}", currentPacket));
            lostPacketCounter++;
        }

        if(deviceDebug)
        {
            debugReport += "CH Status PPS: " + statusLastPPS + "\n";
            debugReport += "Muscle sensor PPS: " + muscleSensorLastPPS + "\n\n";
        }
    }
}