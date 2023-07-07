#include <SoftwareSerial.h>
#include <Servo.h>
Servo buttonServo;

#define startBtn 5 //180 to click
#define lockBtn 6 //0 to click

//sender phone number with country code
const String PHONE = "+12133527873";

#define txPin 11
#define rxPin 12
SoftwareSerial sim800(txPin,rxPin);

String smsStatus,senderNumber,receivedDate,msg;
boolean isReply = false;

void setup() {
  
  Serial.begin(9600);
  Serial.println("Arduino serial initialize");
  //RemoteStart();
  sim800.begin(9600);
  Serial.println("SIM800L software serial initialize");


  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";

  sim800.print("AT+CMGF=1\r"); //SMS text mode
  delay(1000);
  //delete all sms
  sim800.println("AT+CMGD=1,4");
  delay(1000);
  sim800.println("AT+CMGDA= \"DEL ALL\"");
  delay(1000);
  Reply("Remote start System Initiated");
}

void loop() 
{
    while(Serial.available())  
    {
      sim800.println(Serial.readString());
      //Serial.println("got smth2");
    }
    while(sim800.available())
    {
      parseData(sim800.readString());
      //Serial.println("got smth");
    }
} //main loop ends

//***************************************************
void parseData(String buff){
  Serial.println(buff);
  unsigned int len, index;
  
  //Remove sent "AT Command" from the response string.
  index = buff.indexOf("\r");
  buff.remove(0, index+2);
  buff.trim();


  if(buff != "OK"){ //runs every time i send message
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index+2);
    
    if(cmd == "+CMTI"){
      //get newly arrived memory location and store it in temp
      index = buff.indexOf(",");
      String temp = buff.substring(index+1, buff.length()); 
      temp = "AT+CMGR=" + temp + "\r"; 
      //get the message stored at memory location "temp"
      sim800.println(temp); 
      Serial.println("this only happens once----------------------------");
    }
    else if(cmd == "+CMGR"){
      extractSms(buff);
      
      if(senderNumber == PHONE)
      {
        doAction();
        //delete all sms
        sim800.println("AT+CMGD=1,4");
        delay(1000);
        sim800.println("AT+CMGDA= \"DEL ALL\"");
        delay(1000);
      }
      else
      {
        Serial.println("number deosnt match");
      }
    }
  //////////////////////////////////////////////////
  }
  else{
  //The result of AT Command is "OK"
  }
}

//************************************************************
void extractSms(String buff){
   unsigned int index;
   
    index = buff.indexOf(",");
    smsStatus = buff.substring(1, index-1); 
    buff.remove(0, index+2);
    
    senderNumber = buff.substring(0, 12);
    buff.remove(0,19);
   
    receivedDate = buff.substring(0, 20);
    buff.remove(0,buff.indexOf("\r"));
    buff.trim();
    
    index =buff.indexOf("\n\r");
    buff = buff.substring(0, index);
    buff.trim();
    msg = buff;
    buff = "";
    msg.toLowerCase();
}

void doAction(){
  if(msg == "start the car")
  {  
      RemoteStart();
  }
  else if(msg == "lock the car")
  {
    lockTheCar();
    Reply("Car is now locked");
  }
  else if(msg == "turn off remote start")
  {
    stopRemoteStart();
  }
  else if(msg == "start the car w/ response")
  {  
    RemoteStart();
    Reply("The car was started successfully");
  }
  else if(msg == "turn off remote start w/ response"){
    
    Reply("Remote start has now been turned off");
  }
  else if(msg == "blablabla"){
    
    Reply("blablabla");
  }

  
  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";  
}

void Reply(String text)
{
    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\""+PHONE+"\"\r");
    delay(1000);
    sim800.print(text);
    delay(100);
    sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println("SMS Sent Successfully.");
}

void RemoteStart()
{
  LockButton();
  delay(1000);
  StartButton(1500);
  delay(1000);
}

void lockTheCar()
{
  LockButton();
}

void stopRemoteStart()
{
  StartButton(1500);
}

void StartButton(int hold)
{
  buttonServo.attach(startBtn);
  buttonServo.write(120);
  delay(100);
  buttonServo.write(100);
  delay(hold);
  buttonServo.write(80);
  delay(10);
  buttonServo.detach();
}

void LockButton()
{
  buttonServo.attach(lockBtn);
  buttonServo.write(0);
  delay(400);
  buttonServo.write(20);
  delay(200);
//  buttonServo.write(100);
//  delay(30);
  buttonServo.detach();
}

void(* resetFunc) (void) = 0;
