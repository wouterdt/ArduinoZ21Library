#include <SPI.h>
#include <Ethernet.h>
#include <avr/wdt.h>
#include <Z21.h>
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
const int BAUD_RATE = 9600;

IPAddress ip(0,0,0,0);

const IPAddress    Z21_IP(192, 168, 0, 111);
const int          Z21_PORT = 21105;
EthernetClient client;
EthernetUDP Udp;

void poweron(){
    Serial.println("POWER ON");
  }
void poweroff(){
    Serial.println("POWER OFF");
  }
void unknown() {
  
  Serial.println("The Z21 recieved a command that it did not understood");
  
  }
  void switch_changed(uint16_t address, uint8_t accessoryState){
    //counting starts at 0!
    Serial.println("the switch " + String(address) + " is now in " + String(accessoryState));
  }
void setup() {
  wdt_disable();
  // start the serial library:
    Serial.begin(BAUD_RATE);
    Serial.println("Startup");
   if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP, restarting");
      wdt_enable(WDTO_1S);
    delay(1001); //waiting for the dog to bite...
   }
  ip =  Ethernet.localIP();
  Serial.println("DHCP OK ");
  Udp.begin(Z21_PORT);
  //HOOK functions that will be called when a certain packet was recieved by the Z21 class
  Z21.onTrackPowerOn = poweron;  
  Z21.onTrackPowerOff = poweroff;  
  Z21.onUnknownCommand = unknown;
  Z21.onAccessoryInfo = switch_changed;

  //let the library prepare us a status packet and send it to the Z21
  sendPacket(Z21.getStatus()); 
}

void sendPacket(Z21Packet& packet) {
    Udp.beginPacket(Z21_IP, Z21_PORT);
    Udp.write(packet.data, packet.length);
    Udp.endPacket();
}
void loop() {
 Ethernet.maintain(); //DHCP LEASE
  int packetSize = Udp.parsePacket(); 
  if(packetSize)//check if we got something
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i =0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
      char packetBuffer[Z21Packet::MAX_PACKET_SIZE];
      Udp.read(packetBuffer,Z21Packet::MAX_PACKET_SIZE);
      uint8_t read_to [packetSize];
      Serial.println("Contents:");
      for (int i = 0 ; i < packetSize; i++){
          Serial.print("0x");
          Serial.print(packetBuffer[i],HEX);
          Serial.print(" ");
    
          read_to[i] = packetBuffer[i];
            }Serial.println("");
            Serial.println("-------");
            
     if (packetSize < Z21Packet::MIN_PACKET_SIZE || packetSize > Z21Packet::MAX_PACKET_SIZE) {
        Serial.println("will be ignoring packet because it's too small or too big");     
     }else{ //Send data to be processed
              
              delay(10);
              Z21.processPacket(read_to);
      }
     
  }
}
