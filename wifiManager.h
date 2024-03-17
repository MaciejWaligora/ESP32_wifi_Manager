#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <Preferences.h>


class WIFIManager {
private:
    String _client_mode_ssid;
    String _client_mode_psw;
    String _ac_mode_ssid;
    String _ac_mode_psw;
    WiFiServer* _ac_server;
    Preferences* _credentials;
    String _ac_response;
    bool _mode = false; // 1 - Access point mode 0 - client mode
    String parseSSID(String body);
    String parsePSW(String body);
    String buildHTMlTable();
    String addResponseStyle();
    String addJavaScript();


public:
    WIFIManager(String ac_ssid = "ESP32_Network", String ac_psw = "Test1234");
    bool init();
    bool connect(String ssid, String psw);
    bool startAC();
    void acServerLoop();
    void setPermanentCredentials();
    String getPermanentSSID();
    String getPermanentPSW();
    bool tryConnection();
    void clearWifiCredentials();

};
#endif