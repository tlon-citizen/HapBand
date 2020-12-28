
#include <TDConfig.h>
#include <TDTools.h>
#include "WiFiManager.h"
#include "SDUtilities.h"

//IMPORTANT: This line must be changed!!!!!!!!!!!!!!!!!
// C:\Users\ariza\.platformio\packages\framework-arduinoespressif32\tools\sdk\include\lwip\lwip\opt.h
//#define MEMP_NUM_NETBUF 32

WiFiManager::WiFiManager(const char *argLongDesc, const char *argShortDesc) : TDComponent(argLongDesc, argShortDesc, TDCOMPONENT_CONNECTION_BUILTIN)
{
    client = NULL;

    String config[8];
    SDUtilities::loadWiFiConfig(config);

    DEVICE_NAME = config[0];
	LOCAL_IP.fromString(config[1]);
	LOCAL_GATEWAY.fromString(config[2]);
	LOCAL_SUBNET.fromString(config[3]);
	SERVER_IP.fromString(config[4]);
	SERVER_PORT = atoi(config[5].c_str());
	WLAN_SSID = config[6];
	WLAN_PASS = config[7];

    Serial.print("Loaded DEVICE_NAME:   "); Serial.println(config[0]);
    Serial.print("Loaded LOCAL_IP:      "); Serial.println(config[1]);
    Serial.print("Loaded LOCAL_GATEWAY: "); Serial.println(config[2]);
    Serial.print("Loaded LOCAL_SUBNET:  "); Serial.println(config[3]);
    Serial.print("Loaded SERVER_IP:     "); Serial.println(config[4]);
    Serial.print("Loaded SERVER_PORT:   "); Serial.println(config[5]);
    Serial.print("Loaded WLAN_SSID:     "); Serial.println(config[6]);
    Serial.print("Loaded WLAN_PASS:     "); Serial.println(config[7]);
}

void WiFiManager::loop()
{
    if (WiFi.isConnected() && client != NULL && client->connected())
    {
        digitalWrite(led, HIGH);

        if (!credentialsSent) // send credentials to the server
        {
            packet = 'C' + String(DEVICE_NAME) + ',' + WiFi.localIP().toString() + '#';
            client->write(packet.c_str());
            credentialsSent = true;
            if (WIFI_MANAGER_DEBUG) Serial.println(packet.c_str());
        }

        while (client->available())
        {
            delay(1);
            String command;

            try
            {
                command = client->readStringUntil('#');
            }
            catch(const std::exception& e)
            {
                Serial.print("OnReadString Exception : "); 
                Serial.println ( e.what() );
            }

            if(command != NULL && command.length() > 0)
            {
                //ShowLedSequenceD();
                if (WIFI_MANAGER_DEBUG) 
                {
                    Serial.print("RX: "); 
                    Serial.println ( command.c_str() );
                }

                for (rci = remoteCommands.begin(); rci != remoteCommands.end(); ++rci)
                {
                    try
                    {
                        if((*rci)->onCommand(command.c_str())) break;
                    }
                    catch(const std::exception& e)
                    {
                        Serial.print("OnReceive Exception : "); 
                        Serial.println ( e.what() );
                    }
                }
            }
        }
    }
    else
    {
        credentialsSent = false;
        digitalWrite(led, LOW);

        if (!WiFi.isConnected())
        {
            //scanWiFi();
            createWifiConnection(WLAN_SSID.c_str(), WLAN_PASS.c_str(), false);
        }
        else
        {
            if (client != NULL)
            {
                if (WIFI_MANAGER_DEBUG) { Serial.println ( "WiFi : Removing old client..." ); }

                client->stop();
                delete (client);
                client = NULL;
            }

            client = getWifiClient(SERVER_IP, SERVER_PORT);
        }
    }
}

void WiFiManager::sendData(String data)
{
    if (WiFi.isConnected() && client != NULL && client->connected() && credentialsSent)
    {
        size_t size = client->write(data.c_str());
        
        if (WIFI_MANAGER_DEBUG) 
        {
            Serial.print("TX >>> ");  Serial.print(data.c_str()); Serial.print(" : "); Serial.println(size);
        }
    }
}

bool WiFiManager::isWiFiConnected()
{
    return WiFi.isConnected();
}

bool WiFiManager::isClientConnected()
{
    return client != NULL && client->connected();
}

void WiFiManager::addRemoteCommandHandler(WiFiRemoteCommandHandler *handler)
{
    remoteCommands.push_back(handler);
}

bool WiFiManager::createWifiConnection(const char *ssid, const char *pass, bool killPrevious)
{
    retries = 0;
    result = false;

    if (WiFi.isConnected())
    {
        if (killPrevious)
        {
            if (WIFI_MANAGER_DEBUG) { Serial.println ( "WiFi : Killing previous connection..." ); }
            WiFi.disconnect(true);
        }
        else
        {
            result = true;
        }
    }

    if (!WiFi.isConnected())
    {
        if (!WiFi.config(LOCAL_IP, LOCAL_GATEWAY, LOCAL_SUBNET))
        {
            Serial.println("WiFi config failed!");
        }

        WiFi.setAutoReconnect(true);
        WiFi.begin(ssid, pass);
        while (!WiFi.isConnected() && (retries < 10))
        {
            delay(TDCONFIG_WIFI_CONNECT_RETRY_MS);
            Serial.print(".");
            ShowLedSequenceD();
            retries++;
        }
        retries = 0;
        result = WiFi.isConnected();

        if (result)
        {
            Serial.println("");
            Serial.print("WiFi connected >>> Local IP ");
            Serial.println(WiFi.localIP());
            //Serial.print(" SMask >>> ");
            //Serial.print(WiFi.subnetMask());
            //Serial.print(" GW >>> ");
            //Serial.println(WiFi.gatewayIP());
        }
    }

    return (result);
}

WiFiClient *WiFiManager::getWifiClient(IPAddress ip, uint32_t port)
{
    if (!WiFi.isConnected())
    {
        if (WIFI_MANAGER_DEBUG) { Serial.println ( "WiFi : Trying to get a client while disconnected..." ); }
        return (NULL);
    }

    WiFiClient *client = new WiFiClient();
    
    client->connect(ip, port);
    
    delay( TDCONFIG_WIFI_CONNECT_RETRY_MS );

    if(client->connected())
    {
        Serial.println("");
        Serial.print("Connection to host >>> ");
        Serial.print(ip.toString());
        Serial.println(" <<< established!");
    }
    else
    {
        delay( TDCONFIG_WIFI_CONNECT_RETRY_MS * 5 );
        Serial.print("+");
        ShowLedSequenceE();
        delete(client);
        client = NULL;
    }

    return client;
}

void WiFiManager::scanWiFi()
{
    Serial.println("");
    Serial.print("Scan start...");
    int n = WiFi.scanNetworks();
    Serial.println("done.");
    if (n == 0) 
    {
        Serial.println("No networks found");
    } 
    else 
    {
        Serial.print(n);
        Serial.println(" networks found:");
        for (int i = 0; i < n; ++i) 
        {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println(WiFi.encryptionType(i));
            delay(10);
        }
    }
    Serial.println("");
}
