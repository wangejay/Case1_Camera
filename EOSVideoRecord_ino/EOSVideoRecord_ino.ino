#include <usbhub.h>
#include <SPI.h>
#include <ptp.h>
#include <canoneos.h>
#include <MeetAndroid.h>
MeetAndroid meetAndroid;
int inPin = 3;
int val =0 ;
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
    static uint32_t next_time = 0;

    if (!stateConnected)
        stateConnected = true;

    uint32_t  time_now = millis();
    
    //Serial.begin(9600); 
    /*
    if (time_now > next_time)
    {
        next_time = time_now + 5000;

        uint16_t rc = Eos.Capture();

        if (rc != PTP_RC_OK)
            ErrorMessage<uint16_t>("Error", rc);
    }
    */
    
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Start");
    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    //Serial.begin(9600); 
    // register callback functions, which will be called when an associated event occurs.
    meetAndroid.registerFunction(Capture,     'A');
    meetAndroid.registerFunction(SwitchLiveViewOn,   'B');  
    meetAndroid.registerFunction(SwitchLiveViewOff,  'C');   
    meetAndroid.registerFunction(VideoRecord,   'D');   
    meetAndroid.registerFunction(VideoRecordStop,   'E');   
    pinMode(inPin, INPUT);      // sets the digital pin 7 as input
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
    
    if (millis()-KeepAlive_time > 1000)
    {
     meetAndroid.send("keepAlive");
     KeepAlive_time = millis();
    Serial.println(KeepAlive_time); 
    }
    
    //from serial monitor
    if (Serial.available()>0)
    {
      switch (Serial.read())
      {
      case 'A':
        Serial.println("AAAA");
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
  Eos.Capture();
}

void SwitchLiveViewOn (byte flag, byte numOfValues)
{
  Eos.SwitchLiveView(1);
}

void SwitchLiveViewOff (byte flag, byte numOfValues)
{
  Eos.SwitchLiveView(0);
}

void VideoRecord (byte flag, byte numOfValues)
{
  Eos.VideoRecord();
}

void VideoRecordStop (byte flag, byte numOfValues)
{
  Eos.VideoRecordStop();
}
