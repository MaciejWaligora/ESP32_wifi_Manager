#include "wifiManager.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <arduino.h>
/*
 * Cosntructor assigning config 
*/
WIFIManager::WIFIManager(String ac_ssid, String ac_psw, String client_ssid, String client_psw){
    _ac_mode_ssid = ac_ssid;
    _ac_mode_psw = ac_psw;
    _client_mode_ssid = client_ssid;
    _client_mode_psw = client_psw;
    _ac_response = "";
};
/*
* PRivate methods for handlign http response and request
*/
String WIFIManager::parseSSID(String body){
    int index_start = body.indexOf("ssid:")+5;
    int index_end = body.indexOf(";psw:");
    if (index_start != -1 && index_end != -1 && index_start < index_end) {
        return body.substring(index_start, index_end);
    } else {
        return "";
    }
};
String WIFIManager::parsePSW(String body){
    int index_start = body.indexOf(";psw:")+5;
    int index_end = body.length();
    if (index_start != -1 && index_end != -1 && index_start < index_end) {
        return body.substring(index_start, index_end);
    } else {
        return "";
    }
}
String WIFIManager::buildHTMlTable(){
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    String output = "<div class=\"container\"><table><thead><tr><th>ssid:</th><th>rrsi:</th></tr></thead>";
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
        output += "<tr><td>" + WiFi.SSID(i) + "</td><td>" + WiFi.RSSI(i) + "</tr></tr>";
    }
    output += "</table></div>";
    return output;
}
String WIFIManager::addResponseStyle(){
    String output = R"(
        <style>
           /* Styles for the whole page */
            body {
            background-color: #222;
            color: #fff;
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            }

            /* Styles for the container holding the table */
            .container {
            width: 100%;
            padding: 20px;
            box-sizing: border-box; /* Ensures padding is included in width */
            overflow-x: auto; /* Add horizontal scroll if necessary */
            }

            /* Styles for the table */
            table {
            width: 100%;
            border-collapse: collapse;
            }

            /* Styles for table header */
            th {
            background-color: #444;
            color: #fff;
            padding: 10px;
            border: 1px solid #222;
            }

            /* Styles for table cells */
            td {
            padding: 10px;
            border: 1px solid #222;
            }

            /* Center text in the second column */
            td:nth-child(2) {
            text-align: center;
            }

            /* Apply alternating background color to table rows */
            tr:nth-child(even) {
            background-color: #333;
            }

            /* Highlight rows on hover */
            tr:hover {
            background-color: #555;
            }

            /* Make table responsive */
            @media only screen and (max-width: 600px) {
            table, thead, tbody, th, td, tr {
                display: block;
            }
            
            /* Hide table headers */
            thead tr {
                position: absolute;
                top: -9999px;
                left: -9999px;
            }
            
            /* Style table rows */
            tr {
                margin-bottom: 20px;
                border: 2px solid #444;
                display: flex;
                flex-direction: column;
                align-items: stretch;
            }
            
            /* Style table cells */
            td {
                border: none;
                position: relative;
                padding-left: 50%;
                font-size: 18px; /* Adjust font size for mobile */
                text-align: left; /* Ensure text alignment */
            }
            
            /* Style table cell content */
            td::before {
                content: attr(data-label);
                position: absolute;
                left: 0;
                width: 50%;
                padding-left: 15px;
                font-weight: bold;
                font-size: 18px; /* Adjust font size for mobile */
                text-align: left; /* Ensure text alignment */
            }
            }
        </style>
    )";

    return output;
}
String WIFIManager::addJavaScript(){
    String output = R"(
        <script>
            const tableElements = document.getElementsByTagName("tr");
            const rows = [...tableElements];
            rows.shift();

            for (const row of rows) {
                row.childNodes[0].addEventListener("click", (e) => {
                    uploadData(e);
                });
            }

            function uploadData(e) {
                const ssid = e.srcElement.childNodes[0].textContent;
                const password = prompt(`Provide password for ${ssid}`);
                const passphrase = (`ssid:${ssid};psw:${password}`);
                fetch('http://192.168.4.1',{method: "POST", body:`ssid:${ssid};psw:${password}\r\n`})
                    .then((r)=>{
                        console.log(r)
                    });
            }
            function clickHandler() {
                alert("clicked");
            }
        </script>
    )";
    return output;
}

bool WIFIManager::init(){
    _ac_response += buildHTMlTable();
    _ac_response += addResponseStyle();
    _ac_response += addJavaScript();
    startAC();
}
bool WIFIManager::connect(String ssid, String psw){
    _mode = false;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    const char *ssidC = ssid.c_str();
    const char *pswC = psw.c_str();
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssidC);

    WiFi.begin(ssidC, pswC);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
bool WIFIManager::startAC(){
      if(WiFi.softAP(_ac_mode_ssid.c_str(), _ac_mode_psw.c_str())){
           _ac_server = new WiFiServer(80);
           _mode = true;
           IPAddress myIP = WiFi.softAPIP();
           Serial.print("AP name: ");
           Serial.println(_ac_mode_ssid);
           Serial.print("AP password: ");
           Serial.println(_ac_mode_psw);
           Serial.print("GateWay Address: http://");
           Serial.println(myIP);
           _ac_server->begin();
            acServerLoop();
           return true;
      }else{
        return false;
      }
}
void WIFIManager::acServerLoop(){
    while(_mode == true){
        WiFiClient client = _ac_server->available();
        if (client) {
            Serial.println("New Client.");
            String currentLine = "";
            String body = "";
            bool isGet = true;
            bool recordBody = false;
            while (client.connected()) {
                if (client.available()) {
                    char c = client.read();
                    if (c == '\n') {
                        if (currentLine.length() == 0 && isGet) {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-type:text/html");
                            client.println();
                            client.println(_ac_response);
                            client.println();
                            break;
                        }else if(currentLine.length() == 0 && !isGet){
                            recordBody = true;
                        }else if(!isGet && recordBody){
                            _client_mode_ssid = parseSSID(body);
                            _client_mode_psw = parsePSW(body);
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-type:text/html");
                            client.println();
                            client.println("POST Received: \nSSID:" + _client_mode_ssid + "\n Password:" + _client_mode_psw);
                            client.println();
                            connect(_client_mode_ssid, _client_mode_psw);
                            break;
                        }else{
                            currentLine = "";
                        }
                    }else if (c != '\r') {
                        currentLine += c;
                        if(currentLine.indexOf("POST") >= 0){
                            isGet = false;
                        }
                        if (recordBody){
                            body += c;
                        }
                    }
                }
                
            }
            client.stop();
            Serial.println("Client Disconnected.");
        }
    }
}