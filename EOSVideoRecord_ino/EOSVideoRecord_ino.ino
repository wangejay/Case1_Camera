#include <usbhub.h>
#include <SPI.h>
#include <ptp.h>
#include <canoneos.h>
#include <MeetAndroid.h>
#include <EEPROM.h>

MeetAndroid meetAndroid;
//int inPin = 3;
int testPin = 7; 
int val =0 ;
int device_number=0; 
PTPReadParser *parser;

uint32_t KeepAlive_time =0;

class CamStateHandlers : public PTPStateHandlers
{
      bool stateConnected;

public:
      CamStateHandlers() : stateConnected(false) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);        
} CamStates;

USB         Usb;
USBHub      Hub1(&Usb);
CanonEOS    Eos(&Usb, &CamStates);

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected)
    {
        stateConnected = false;
        E_Notify(PSTR("Camera disconnected\r\n"),0x80);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    
    if (!stateConnected)
        stateConnected = true;
    
}

void(* resetFunc) (void) = 0; //reset funciton

void setup()
{
    Serial.begin(9600);
    Serial.print("Start V1.1");
    device_number = EEPROM.read(4);
    Serial.print("\t");
    Serial.print(device_number);
    Serial.println();
    
    Serial.println("INIT START");
    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    Serial.println("INIT END");
    
    //Serial.begin(9600); 
    // register callback functions, which will be called when an associated event occurs.
    meetAndroid.registerFunction(Capture,     'A');
    meetAndroid.registerFunction(SwitchLiveViewOn,   'B');  
    meetAndroid.registerFunction(SwitchLiveViewOff,  'C');   
    meetAndroid.registerFunction(VideoRecord,   'D');   
    meetAndroid.registerFunction(VideoRecordStop,   'E'); 
    meetAndroid.registerFunction(USB_init,   'I'); 
    
    //pinMode(inPin, INPUT);      // sets the digital pin 7 as input
    pinMode(testPin, OUTPUT);      // sets the digital pin 7 as input
    delay( 200 );
}

void loop()
{
    //Serial.println("Loop");
    Usb.Task();
    //Serial.println("test");
    //val = digitalRead(inPin);
    //Serial.println(val);
    //delay (100);
    //Enable BT
    meetAndroid.receive();
    
    if (millis()-KeepAlive_time > 10000)
    {
     
     //meetAndroid.send(device_number);
     //Serial.println("\t");
     meetAndroid.send("keepAlive");
     KeepAlive_time = millis();
     //Serial.println();
     //Serial.println(KeepAlive_time);
     Eos.GetDeviceInfoEx(parser);
    }
    
    //from serial monitor
    if (Serial.available()>0)
    {
      switch (Serial.read())
      {
      case 'A':
        Serial.println("A");
        Capture(0,0); 
        break; 
      
      case 'B':
        SwitchLiveViewOn(0,0); 
        break;
      
      case 'C':
        SwitchLiveViewOff(0,0); 
        break;
      
      case 'D':
        VideoRecord(0,0); 
        break;
        
      case 'E':
        VideoRecordStop(0,0); 
        break;
        
      }
    }
}

void Capture (byte flag, byte numOfValues)
{
  uint16_t rc = Eos.Capture();
  if (rc != PTP_RC_OK)
     {
     Serial.println("Arduino reset");  
     resetFunc(); //call reset 
     }
  digitalWrite(testPin,HIGH); 
  delay (100);
  digitalWrite(testPin,LOW); 
  meetAndroid.send("A OK");  
}

void SwitchLiveViewOn (byte flag, byte numOfValues)
{
  Eos.SwitchLiveView(1);
  meetAndroid.send("B OK");
}
  
void SwitchLiveViewOff (byte flag, byte numOfValues)
{
  Eos.SwitchLiveView(0);
  if (Usb.Init() == -1)
        Serial.println("OSC did not start.");
  meetAndroid.send("B OK");
}

void VideoRecord (byte flag, byte numOfValues)
{
  Eos.VideoRecord();
  digitalWrite(testPin,HIGH);
  meetAndroid.send("D OK");
}

void VideoRecordStop (byte flag, byte numOfValues)
{
  Eos.VideoRecordStop();
  digitalWrite(testPin,LOW);
  meetAndroid.send("E OK");
}

void USB_init (byte flag, byte numOfValues)
{
  if (Usb.Init() == -1)
        Serial.println("OSC did not start.");
  meetAndroid.send("I OK");
}
