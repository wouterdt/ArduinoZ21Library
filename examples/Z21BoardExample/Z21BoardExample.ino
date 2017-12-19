#include <SPI.h>
#include <Ethernet.h>
#include <avr/wdt.h>
#include <Z21.h>
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
const int BAUD_RATE = 9600;
const int INPUTBEGIN=A8;
const int INPUTEND=A14;
const int OUTPUTBEGIN=A0;
const int OUTPUTEND=A3;

//variables indicating array size must be const and at array declaration need to be 1 bigger so we add 1
//we will perform a < operation on when looping the array
const int inputarraysize = INPUTEND - INPUTBEGIN +1;
const int outputarraysize = OUTPUTEND - OUTPUTBEGIN+1;
const uint8_t pos1 = Z21_Accessory_Pos::P1;
const uint8_t pos0 = Z21_Accessory_Pos::P0;
//rows contain the input pins
//columns contain the output
//We number according to the Roco standard starting at 1
//we will deduct 1 when we send it to Z21, so that numbers in this array correspond with what is displayed on multimaus
uint8_t Buttons[inputarraysize][outputarraysize][2] = { 
{ {15,pos1},{15,pos0},{8,pos0},{8,pos1} },
{ {13,pos1},{14,pos0},{7,pos0},{6,pos1} },
{ {13,pos0},{14,pos1},{7,pos1},{5,pos1} },
{ {11,pos1},{10,pos0},{2,pos0},{5,pos0} },
{ {11,pos0},{10,pos1},{2,pos1},{4,pos1} },
{ {9,pos1},{1,pos1},{3,pos0},{4,pos0} },
{ {9,pos0},{1,pos0},{3,pos1},{6,pos0} },
};
//convert the input pin number to the position in the button array
//used to find the address and position in the array
uint8_t pintoinputarray(uint8_t pin){
    return pin - INPUTBEGIN;
}
uint8_t pintooutputarray(uint8_t pin){
    return pin - OUTPUTBEGIN;
}
uint8_t outputarraytopin(uint8_t pos){
  return pos + OUTPUTBEGIN;
  }

uint8_t inputarraytopin(uint8_t pos){
  return pos + INPUTBEGIN;
  }
IPAddress ip(0,0,0,0);

const IPAddress    Z21_IP(192, 168, 0, 111);
const int          Z21_PORT = 21105;
EthernetClient client;
EthernetUDP Udp;
boolean state;
  void poweron(){
    Serial.println("POWER ON");
  }
  void poweroff(){
    Serial.println("POWER OFF");
  }
  void unknown() {
    Serial.println("The Z21 recieved a command that it did not understand");
  }
  void switch_changed(uint16_t address, uint8_t accessoryState){
    Serial.println("the switch " + String(address+1) + " is now in P" + String(accessoryState-1));
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
  //hooking in callback functions that will be called when a certain packet was recieved by the Z21 class
  Z21.onTrackPowerOn = poweron;  
  Z21.onTrackPowerOff = poweroff;  
  Z21.onUnknownCommand = unknown;
  Z21.onAccessoryInfo = switch_changed;
for(int u = OUTPUTBEGIN; u<=OUTPUTEND; u++){
    pinMode(u,OUTPUT);
    digitalWrite(u,LOW);
    
  }
for(int i = INPUTBEGIN; i<=INPUTEND; i++){
    pinMode(i,INPUT);
  
}
  //let the library prepare us a status packet and send it to the Z21
  sendPacket(Z21.getStatus()); 
  state = false;

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
       for(int o = 0; o<=outputarraysize; o++){
         digitalWrite(outputarraytopin(o), HIGH);//turn on the row;
         //Serial.println("5 Volt on outputpin " + String(outputarraytopin(o)) ); 
          for(int i = 0; i<=inputarraysize;i++){
             //Serial.println("reading inputpin " + String(inputarraytopin(i)) ); 
                if (digitalRead(inputarraytopin(i)) == HIGH){
                  //Serial.println("reading from array input pos " + String(i));
                 // Serial.println("reading from array output pos " +String(o));
                 // Serial.println("PIN ON: " + String(outputarraytopin(i)) );
                  Serial.println("Activating switch " + String(Buttons[i][o][0]) + " to positon P" + String(Buttons[i][o][1]));
                                                //-1 here as 1 displayed on multimause becomes 0 for Z21 
                    sendPacket(Z21.setAccessory(Buttons[i][o][0]-1,Buttons[i][o][1],true,false)); //activate
                    delay(100);
                   Serial.println("deactivating switch " + String(Buttons[i][o][0]) + " to positon P" + String(Buttons[i][o][1]));
                    sendPacket(Z21.setAccessory(Buttons[i][o][0]-1,Buttons[i][o][1],false,false)); //deactivate 
                  boolean wasreleased = false;
                  delay(50);

                  while (wasreleased == false){
                    Serial.println("Waiting for button to be released");
                  if (digitalRead(inputarraytopin(i)) == LOW){
                      wasreleased = true;
                        Serial.println("Button released");

                    }  
                  }
                }
            }
        digitalWrite(outputarraytopin(o), LOW);//turn on the row;
        }
}
