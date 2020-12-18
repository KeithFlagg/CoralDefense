//level switch high signal, shut down fresh water pump and turn of uv bar
//level switch low turn on uv sterilization and fresh water pump
#include <dht.h> //Library needed for humidity and temperature sensors
#include <stdlib.h>
#define uv_pin 23
#define he_pin 25
#define dht_pin 4
#define fs_pin 2
#define sensor_interrupt 0 //
#define ls_pin 20
#define led_pin 53
#define fwfp_pin 22
//defining device pins
#define bt Serial1 //HC-05 bluetooth chip \
                   //all the information relating to the flow sensors
float flowrate;
float volumeml;
unsigned long totalml;
unsigned long oldtime;
volatile byte pulsecount;
float calibration = 4.5;
//make a dht sensor object to read humidity and temperature
dht DHT;
int bt_error(int); //error handlers
int inboard_error(int);
int get_flow_info();                  //get all the information regarding the  flow
int package_bundle();                 // gather data for and formate the bublde string return negative if error
int send_bundle();                    //send the bundle over the bt connection return negative if error
int recieve_data();                   //read the bundle from the bt connection into the reader, return negative if error
int get_he_state();                   //get hall effect state and return
int get_ls_state();                   //get level switch state and return
void turn_off_UV();                   //turns off the uv light does not return a value because the call to the pin is void
void turn_on_UV();                    //turns on the uv light does not return a value because the call to the pin is void
int state_check(int);                 //state handler for recieved data
float get_humidity();                 //read humidity and return
float get_temperature();              //read temperature and return
void turn_on_fwf_pump();              //turns on the fresh water fill pump
void turn_off_fwf_pump();             //turns off the freshwater fill pump
char *convert_float_to_string(float); //converts a float to a string
void pulseCounter();                  //used to count pulses, keeps time for flowinfo
char sender[255];                     // string to be formatted and sent over bluetooth
int receiver = -1;                    //int that gets read from the phone
char *arr[5];                         //string array for formatting sender
void setup()
{

  pinMode(led_pin, OUTPUT);
  pinMode(ls_pin, INPUT_PULLUP);
  pinMode(he_pin, INPUT);
  pinMode(uv_pin, OUTPUT);
  pinMode(fs_pin, INPUT);
  Serial.begin(9600);
  bt.begin(9600);
  flowrate = 0;
  volumeml = 0;
  totalml = 0;
  oldtime = 0;
  pulsecount = 0;
  attachInterrupt(sensor_interrupt, pulseCounter, FALLING); //interrupt when voltage falls, and run the function pulseCounter
}

void loop()
{
  int packagecheck, sendcheck, readcheck;
  packagecheck = package_bundle();
  if (packagecheck < 0)
  {
    Serial.println("Packaging error");
    Serial1.println("Packaging error");
  }
  sendcheck = send_bundle();
  if (sendcheck < 0)
  {
    Serial.println("Sending error");
    bt.println("Sending error");
  }
  delay(1700);
  readcheck = recieve_data();
  if (readcheck < 0)
  {
    Serial.println("Nothing new read");
    bt.println("Nothing new read");
  }
  if (readcheck > 0) //there is a passed number from phone that is not a error
  {
    int stater;
    stater = state_check(readcheck);
    if (stater < 0)
    {
      inboard_error(stater);
    }
  }
  if (readcheck == 0)
  {
    Serial.println("No message sent");
  }
  delay(1700);
}
void pulseCounter()
{
  pulsecount++;
}
char *convert_float_to_string(float n)
{
  char *mynum = new char[5];
  dtostrf(n, 3, 2, mynum);
  return mynum;
}
float get_humidity()
{

  DHT.read11(dht_pin);
  float humidity = DHT.humidity;
  Serial.print("HUM:");
  Serial.print(humidity);
  Serial.print("\n");
  return humidity;
}
float get_temperature()
{
  DHT.read11(dht_pin);
  float temperature = DHT.temperature;
  Serial.print("TEMP:");
  Serial.print(temperature);
  Serial.print("\n");
  return temperature;
}
int get_he_state()
{
  int hestate = digitalRead(he_pin);
  Serial.print("HESTATE:");
  Serial.print(hestate);
  Serial.print("\n");

  return hestate;
}
void turn_off_UV()
{
  Serial.println("Turning off UV light");
  digitalWrite(uv_pin, HIGH);
}
void turn_on_UV()
{
  Serial.println("Turning off UV light");
  digitalWrite(uv_pin, LOW);
}
int get_uv_state()
{
  int uvstate = digitalRead(uv_pin);
  Serial.print("UVSTATE:");
  Serial.print(uvstate);
  Serial.print("\n");
}
int get_ls_state()
{
  int lsstate = digitalRead(ls_pin);
  Serial.print("LSSTATE:");
  Serial.print(lsstate);
  Serial.print("\n");
  return lsstate;
}
int get_flow_info()
{
  if ((millis() - oldtime > 1000))
  {
    detachInterrupt(sensor_interrupt);
    flowrate = ((1000.0 / (millis() - oldtime)) * pulsecount) / calibration;
    oldtime = millis();
    volumeml = (flowrate / 60) * 1000;
    totalml += volumeml;
    Serial.print("Flow Rate:");
    Serial.print(flowrate);
    Serial.print("mL/min\n");
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalml);
    Serial.print("mL");
    Serial.print("\t"); // Print tab space
    Serial.print(totalml / 1000);
    Serial.print("L");
    Serial.print("\n");
    pulsecount = 0;
    attachInterrupt(sensor_interrupt, pulseCounter, FALLING);
  }
  else
  {
    Serial.println("Flow sensor Error");
    inboard_error(-25);
    return -25;
  }
}
void turn_on_fwf_pump()
{
  digitalWrite(fwfp_pin, HIGH);
}
void turn_off_fwf_pump()
{
  digitalWrite(fwfp_pin, LOW);
}
int package_bundle()
{
  get_flow_info();
  arr[0] = convert_float_to_string(get_humidity());
  arr[1] = convert_float_to_string(get_temperature());
  arr[2] = convert_float_to_string(flowrate);
  arr[3] = convert_float_to_string(totalml);
  arr[4] = convert_float_to_string(volumeml);
  int n = sprintf(sender,
                  "Humidity:%s#\nTemperature:%s#\nFlow_Frequency:%s#\nFlow_totalml:%s#\nFlow_volumeml:%s#\nHall_Effect_State:%d#\nLevel_Switch_State:%i#\nUV_State %d#\n",
                  arr[0], arr[1], arr[2], arr[3], arr[4], get_he_state(), get_ls_state(), get_uv_state());
  Serial.println(sender);
  if (n < 0)
  {
    return -14;
  }
  else
    return 0;
}
int send_bundle()
{
  if (bt.availableForWrite())
  {
    Serial.println("Sending String:");
    Serial.print(sender);
    bt.println(sender);
    return 0;
  }
  else
    return -15;
}
int recieve_data()
{
  int n = receiver;
  if (bt.available())
  {
    receiver = bt.read();
    if (n != receiver)
    {
      Serial.println(receiver);
      return receiver;
    }
    else
    {
      return 0;
    }
  }
  else
    Serial.println(n);
  return -12;
}
int state_check(int phonedata)
{
  switch (phonedata)
  {
  case 112:
    turn_on_UV();
    return 20;
    break;
  case 113:
    turn_off_UV();
    return 21;
    break;
  case 114:
    turn_on_fwf_pump();
    return 22;
    break;
  case 115:
    turn_off_fwf_pump();
    return 22;
    break;
  default:
    return -20;
    break;
  }
}
int bt_error(int code)
{
  char message[21];
  sprintf(message, "Error code %i#", code);
  if (bt.availableForWrite() > 0)
  {
    bt.println(message);
    return 0;
  }
  else
  {
    Serial.println("Error with the error handler sending over bluetooth");
    return -52;
  }
}
int inboard_error(int code)
{
  char message[21];
  sprintf(message, "Error code %i#", code);
  if (bt.availableForWrite() > 0)
  {
    Serial.println(message);
    bt.println(message);
    return 0;
  }
  else
  {
    Serial.println("Error with the error handler sending over bluetooth");
    Serial.println(message);
    return -53;
  }
}
