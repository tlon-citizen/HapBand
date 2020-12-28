
/**
 *  Haptics Framework
 *
 *  UHH HCI 
 *  Author: Oscar Ariza <ariza@informatik.uni-hamburg.de>
 *
 */

/// Adapted from https://github.com/hardlydifficult/MultiplayerGameProgramming

using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using UnityEngine;

// Notes:

// Finde the firewall rule blovking connection:
// https://superuser.com/questions/1130078/how-to-tell-which-windows-firewall-rule-is-blocking-traffic

public class DeviceServer : MonoBehaviour
{
    private TcpListener tcpListener;
    private TcpClient incomingTCPDevice;
    private readonly byte[] readBuffer = new byte[100];
    private int readLength;

    public Dictionary<String, AbstractDevice> connectedDevicesByName = new Dictionary<String, AbstractDevice>();
    
    public static DeviceServer instance;
    public int serverPort = 23;
    [SerializeField]
    public List<AbstractDevice> availableDevices = new List<AbstractDevice>();
    
    public void Awake()
    {
        instance = this;

        PrintLocalIPAddresses();

        tcpListener = new TcpListener(IPAddress.Any, serverPort);
        tcpListener.Start();
        tcpListener.BeginAcceptTcpClient(OnServerConnect, null);
        
        Debug.Log("Server listening on port " + serverPort);
    }

    public void OnDestroy()
    {
        if (tcpListener != null)
        {
            tcpListener.Stop();
            tcpListener = null;
        }

        Debug.Log("Server destroyed!");
    }

    void OnServerConnect(IAsyncResult ar)
    {
        if (tcpListener == null) return;
        incomingTCPDevice = tcpListener.EndAcceptTcpClient(ar);
        //incomingTCPDevice.NoDelay = true; // Disable Nagle's cache algorithm
        incomingTCPDevice.GetStream().BeginRead(readBuffer, 0, readBuffer.Length, OnRead, null);
        tcpListener.BeginAcceptTcpClient(OnServerConnect, null);
    }
    
    void OnRead(IAsyncResult ar)
    {
        readLength = incomingTCPDevice.GetStream().EndRead(ar);
        if (readLength <= 0) return;
        string packet = System.Text.Encoding.UTF8.GetString(readBuffer, 0, readLength);
        bool credentialsOk = CheckCredentials(packet);
        if(!credentialsOk) incomingTCPDevice.GetStream().BeginRead(readBuffer, 0, readBuffer.Length, OnRead, null);
    }

    private bool CheckCredentials(string packet)
    {
        int parsingIndex = packet.IndexOf('#', 0);
        bool credentialsOk = false;

        if (parsingIndex != -1)
        {
            packet = packet.Substring(0, parsingIndex);
            char parsingToken = packet[0];
            packet = packet.TrimStart(packet[0]);
            string[] packetValues = packet.Split(',');

            if (parsingToken == 'C' && packetValues.Length == 2)
            {
                credentialsOk = true;
                string deviceName = packetValues[0];
                string deviceIP = packetValues[1];
                bool deviceFound = false;

                foreach (AbstractDevice device in availableDevices)
                {
                    if (device.GetDeviceName().Equals(deviceName))
                    {
                        deviceFound = true;
                        AddConnectedDevice(device);
                        device.Enable(incomingTCPDevice, deviceIP);
                    }
                }

                if (!deviceFound)
                {
                    Debug.Log("The incoming device " + deviceName + " is not registered as available in the server!");
                }
            }
        }

        return credentialsOk;
    }
    
    public void AddConnectedDevice(AbstractDevice device)
    {
        bool reconnected = false;

        if (connectedDevicesByName.ContainsKey(device.GetDeviceName()))
        {
            connectedDevicesByName.Remove(device.GetDeviceName());
            reconnected = true;
        }
        
        connectedDevicesByName.Add(device.GetDeviceName(), device);
        Debug.Log("Device " + device.GetDeviceName() + ((reconnected) ? " re" : " ") + "connected");
    }

    public void RemoveDevice(AbstractDevice device)
    {
        connectedDevicesByName.Remove(device.GetDeviceName());
    }

    protected void OnApplicationQuit()
    {
        tcpListener?.Stop();
        foreach (AbstractDevice device in connectedDevicesByName.Values)
        {
            device.CloseConnection();
        }
    }
    
    internal void SendCommand(string deviceName, string command)
    {
        if (!instance.connectedDevicesByName.ContainsKey(deviceName)) return;

        connectedDevicesByName[deviceName].SendCommand(command);
    }

    public void PrintLocalIPAddresses()
    {
        IPHostEntry host;
        string localIPs = "";
        host = Dns.GetHostEntry(Dns.GetHostName());
        foreach (IPAddress ip in host.AddressList)
        {
            if (ip.AddressFamily == AddressFamily.InterNetwork)
            {
                localIPs += ip.ToString() + " ";
            }
        }
        Debug.Log("Local IP(s): " + localIPs);
    }

    protected void Update()
    {
        /*
        foreach (AbstractDevice device in connectedDevicesByName.Values)
        {
            // update devices here
        }
        */
    }
}