using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EMSManager : MonoBehaviour
{
    [SerializeField] public DeviceServer deviceServer;

    [Range(0, 300)] public byte CH1PeakDuration = 200;
    [Range(0, 100)] public byte CH1PeakIntensity = 40;
    [Range(-100, 100)] public short CH1IntensityDelta = -5;

    [Range(0, 300)] public byte CH2PeakDuration = 200;
    [Range(0, 100)] public byte CH2PeakIntensity = 40;
    [Range(-100, 100)] public short CH2IntensityDelta = -5;
    
    private EMSUnit emsUnit = null;

    [HideInInspector] public string deviceName;

    public void Start()
    {
        deviceName = FindObjectOfType<EMSUnit>().deviceName;
    }

    void Update ()
    {
        if (emsUnit == null && deviceServer.connectedDevicesByName.ContainsKey(deviceName))
        {
            emsUnit = (EMSUnit)deviceServer.connectedDevicesByName[deviceName];
        }

        if (Input.GetKeyUp(KeyCode.F1))
        {
            CH1Off();
            CH2Off();
        }
        else if (Input.GetKeyUp(KeyCode.F2))
        {
            CH1SendPeakTriggerCommand();
            CH2SendPeakTriggerCommand();
        }
    }

    // Commands

    public void CH1On()
    {
        if (emsUnit == null) return;
        emsUnit.SendCommand("C1E");
    }

    public void CH2On()
    {
        if (emsUnit == null) return;
        emsUnit.SendCommand("C2E");
    }

    public void CH1Off()
    {
        if (emsUnit == null) return;
        emsUnit.SendCommand("C1D");
    }

    public void CH2Off()
    {
        if (emsUnit == null) return;
        emsUnit.SendCommand("C2D");
    }

    public void CH1SetIntensity()
    {
        if (emsUnit == null) return;
        emsUnit.SendCommand("C1I" + CH1IntensityDelta);
    }

    public void CH2SetIntensity()
    {
        if (emsUnit == null) return;
        emsUnit.SendCommand("C2I" + CH2IntensityDelta);
    }

    public void CH1SendPeakTriggerCommand()
    {
        if (emsUnit == null) return;
        emsUnit.SendCommand("C1P" + CH1PeakDuration + "," + CH1PeakIntensity);
    }

    public void CH2SendPeakTriggerCommand()
    {
        if (emsUnit == null) return;
        emsUnit.SendCommand("C2P" + CH2PeakDuration + "," + CH2PeakIntensity);
    }
}
