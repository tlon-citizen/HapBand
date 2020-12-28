
/**
 *  Haptics Framework
 *
 *  UHH HCI 
 *  Author: Oscar Ariza <ariza@informatik.uni-hamburg.de>
 *
 */

/// Adapted from https://github.com/hardlydifficult/MultiplayerGameProgramming

using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

public abstract class AbstractDevice : MonoBehaviour
{
    [SerializeField]
    protected string IP;
    protected string packetBuffer;
    protected long packetCounter = 0;
    protected string[] packetValues;
    protected string currentPacket;
    protected long lostPacketCounter = 0;
    protected char parsingToken;
    protected TcpClient tcpConnection;

    public string deviceName;
    public bool deviceDebug = !false;
    [TextArea(4, 4)] public string debugReport; 

    private byte[] readBuffer = new byte[100];
    private int readLength;
    private int parsingIndex;
    private byte[] commandBuffer;
    private bool selfClosed;

    public void Enable(TcpClient tcp, string deviceIP)
    {
        IP = deviceIP;
        tcpConnection = tcp;
        tcpConnection.GetStream().BeginRead(readBuffer, 0, readBuffer.Length, OnRead, null);
        selfClosed = false;
    }

    void OnRead(IAsyncResult ar)
    {
        readLength = tcpConnection.GetStream().EndRead(ar);

        if (readLength <= 0)
        {
            Debug.Log("Device " + deviceName + " disconnected");
            return;
        }

        packetBuffer = System.Text.Encoding.UTF8.GetString(readBuffer, 0, readLength);

        string[] packets = packetBuffer.Split('#');

        foreach (var packet in packets)
        {
            packetCounter++;
            currentPacket = packet;

            if (currentPacket.Length <= 1) break;

            if (deviceDebug)
            {
                debugReport =
                "Remote IP: " + IP + "\n" +
                "Packets (R/D): " + packetCounter + "/" + lostPacketCounter + "\n";
            }

            ProcessPacket();
        }

        tcpConnection.GetStream().BeginRead(readBuffer, 0, readBuffer.Length, OnRead, null);
    }

    //void OnRead(IAsyncResult ar)
    //{
    //    readLength = tcpConnection.GetStream().EndRead(ar);

    //    if (readLength <= 0)
    //    {
    //        Debug.Log("Device " + deviceName + " disconnected");
    //        return;
    //    }

    //    packetBuffer = System.Text.Encoding.UTF8.GetString(readBuffer, 0, readLength);

    //    parsingIndex = packetBuffer.IndexOf('#', 0);
    //    if (parsingIndex != -1)
    //    {
    //        packetCounter++;
    //        currentPacket = packetBuffer.Substring(0, parsingIndex);
    //        packetBuffer = packetBuffer.Substring(parsingIndex + 1, packetBuffer.Length - parsingIndex - 1);

    //        if (deviceDebug)
    //        {
    //            debugReport =
    //            "Remote IP: " + IP + "\n" +
    //            "Packets (R/D): " + packetCounter + "/" + lostPacketCounter + "\n" +
    //            "Buffer (" + packetBuffer.Length + "): " + packetBuffer + "\n";
    //        }

    //        ProcessPacket();
    //    }

    //    tcpConnection.GetStream().BeginRead(readBuffer, 0, readBuffer.Length, OnRead, null);
    //}


    internal void CloseConnection()
    {
        Debug.Log("Device " + deviceName + " >>> Connection closed by server");
        tcpConnection.Close();
        selfClosed = true;
    }

    public string GetDeviceName()
    {
        return deviceName;
    }

    public void SendCommand(string command)
    {
        if (tcpConnection == null || !tcpConnection.Connected)
        {
            Debug.Log("Trying to send a command with a closed TCP connection!");
            return;
        }

        command += "#";
        commandBuffer = System.Text.Encoding.UTF8.GetBytes(command);
        tcpConnection.GetStream().Write(commandBuffer, 0, commandBuffer.Length);
        if (deviceDebug) Debug.Log("Command send: " + command);
    }

    public void Update()
    {
        if (selfClosed && (tcpConnection == null || !tcpConnection.Connected))
        {
            Debug.Log("Device " + deviceName + " >>> Connection closed by device");
            tcpConnection.Close();
            tcpConnection = null;
        }
    }

    public abstract void ProcessPacket();
}