#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <TDComponent.h>
#include <TDTools.h>
#include <TDConfig.h>
#include <WiFi.h>
#include <list>

class WiFiRemoteCommandHandler
{
  public:
    virtual bool onCommand(String);
};

class WiFiManager : public TDComponent
{
  public:
    WiFiManager(const char *argLongDesc, const char *argShortDesc);
    void loop();
    void sendData(String data);
    bool isWiFiConnected();
    bool isClientConnected();
    void addRemoteCommandHandler(WiFiRemoteCommandHandler *handler);

  private:
    WiFiClient *getWifiClient(IPAddress ip, uint32_t port);
    bool createWifiConnection(const char *ssid, const char *pass, bool killPrevious);
    void scanWiFi();
    WiFiClient *client;
    std::list<WiFiRemoteCommandHandler*> remoteCommands;
    std::list<WiFiRemoteCommandHandler*>::iterator rci;
    int retries;
    bool credentialsSent = false;
    bool result;
    String packet;
    String DEVICE_NAME;
	IPAddress LOCAL_IP;
	IPAddress LOCAL_GATEWAY;
	IPAddress LOCAL_SUBNET;
	IPAddress SERVER_IP;
	uint32_t SERVER_PORT;
	String WLAN_SSID;
	String WLAN_PASS;
};

#endif // WIFI_MANAGER_H
