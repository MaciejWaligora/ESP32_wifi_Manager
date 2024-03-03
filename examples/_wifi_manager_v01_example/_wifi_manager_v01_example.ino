#include <wifiManager.h>

/*
*  Initialize WifiManager class, you can provide deafult ssid and pasw
*  (for access point adn wifi client as constructor arguments) 
*/
WIFIManager wifi("MyAccessPint", "MyACPassword", "ClientSSIDname", "CleitnSSIDPassword");


/*
*  Call init() during the setup(), once wifi manager flow finalizes the code will proceed
*/
void setup(){
	Serial.begin(115200);
    wifi.init();
    //The rest of your setup
}

void loop(){

    //Your loop code

}
