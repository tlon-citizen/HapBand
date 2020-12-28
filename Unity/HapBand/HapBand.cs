
using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEditor;
using UnityEngine;

[Serializable]
public class HapBand : AbstractDevice
{
    [HideInInspector] public bool notifications = false;
    [HideInInspector] public float pressureSensor = 0;
    [HideInInspector] public bool capacitiveSensor1 = false;
    [HideInInspector] public bool capacitiveSensor2 = false;
    [HideInInspector] public bool capacitiveSensor3 = false;

    private static System.Diagnostics.Stopwatch sw = System.Diagnostics.Stopwatch.StartNew();

    private long notificationsT1 = sw.ElapsedMilliseconds;
    private long notificationsT2 = 0;
    private long notificationsPacketCounter;
    private long notificationsLastPPS;

    private byte capacitiveStatus = 0;

    public void ToggleNotifications()
    {
        notifications = !notifications;
        SendCommand("NED" + (notifications ? "1" : "0"));
    }

    public override void ProcessPacket()
    {
        try
        {
            parsingToken = currentPacket[0];
            currentPacket = currentPacket.TrimStart(parsingToken);
            packetValues = currentPacket.Split(',');

            if (parsingToken == 'N' && packetValues.Length == 2)
            {
                pressureSensor = float.Parse(packetValues[0]);
                capacitiveStatus = byte.Parse(packetValues[1]);

                capacitiveSensor1 = (capacitiveStatus & (1 << 7)) > 0;
                capacitiveSensor2 = (capacitiveStatus & (1 << 6)) > 0;
                capacitiveSensor3 = (capacitiveStatus & (1 << 5)) > 0;

                if (deviceDebug)
                {
                    notificationsT2 = sw.ElapsedMilliseconds;
                    notificationsPacketCounter++;
                    if (notificationsT2 - notificationsT1 >= 1000)
                    {
                        notificationsLastPPS = notificationsPacketCounter;
                        notificationsT1 = notificationsT2;
                        notificationsPacketCounter = 0;
                    }
                }
            }
            else
            {
                Debug.Log(string.Format("Lost Packet >>> {0}", currentPacket));
                lostPacketCounter++;
            }

            if (deviceDebug)
            {
                debugReport += "Notifications PPS: " + notificationsLastPPS + "\n\n";
            }
        }
        catch (Exception e)
        {
            Debug.Log("Exception parsing package: " + e.Message);
        }
    }

    public string PrintCapacitiveStatus()
    {
        return (capacitiveSensor1 ? "1" : "0") + (capacitiveSensor2 ? "1" : "0") + (capacitiveSensor3 ? "1" : "0");
    }
}