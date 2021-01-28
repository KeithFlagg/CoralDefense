//level switch high signal, shut down fresh water pump and turn of uv bar
//level switch low turn on uv sterilization and fresh water pump
#include <dht.h> //Library needed for humidity and temperature sensors
#include <stdlib.h>
#define bt_state 37
#define uv_pin 23
#define he_pin 25
#define dht_pin 4
#define fs_pin 2
#define sensor_interrupt 0 
#define ls_pin 20
#define led_pin 53
#define fwfp_pin 22
//defining device pins
#define bt Serial1                       //HC-05 bluetooth chip 
                   //all the information relating to the flow sensors
float flow_rate_ml;                      //keeps track of flow rate
float flow_volume_ml;                    //keeps track of volume
unsigned long flow_total_ml;             //keeps track of the total amount of flow
unsigned long old_flow_time;             //keeps track of time passed for the flow function
volatile byte flow_sensor_pulse_count;   //keeps track of the pulses the flow sensor has sent
float flow_sensor_tick_rate = 1;         //flow_sensor_tick_rate on the flow sensor
const long recieve_time_trigger = 1000;  //triggers after 1000 ms to run code
const long package_time_trigger = 4000;  //triggers after 4000 ms to run code
unsigned long recieve_previous_time = 0; //keeps time before the recieve_data code runs
unsigned long package_previous_time = 0; //keeps time before the package and send code runs
dht DHT;                                 //make a dht sensor object to read humidity and temperature
int bt_error(int);                       //error handlers
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
void flow_sensor_pulse_counter();     //used to count pulses, keeps time for flowinfo
char sender[255];                     // string to be formatted and sent over bluetooth
int receiver = 0;                     //int that gets read from the phone
char *arr[5];                         //string array for formatting sender
void setup()
{
  pinMode(bt_state,INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(ls_pin, INPUT_PULLUP);
  pinMode(he_pin, INPUT);
  pinMode(uv_pin, OUTPUT);
  pinMode(fs_pin, INPUT);
  Serial.begin(9600);
  bt.begin(9600);
  flow_rate_ml = 0;
  flow_volume_ml = 0;
  flow_total_ml = 0;
  old_flow_time = 0;
  flow_sensor_pulse_count = 0;
  attachInterrupt(sensor_interrupt, flow_sensor_pulse_counter, FALLING); //interrupt when voltage falls, and run the function pulseCounter
}

void loop()
{
  int packagecheck, sendcheck, readcheck;
  unsigned long current_time = millis();
  //recieve data timer
  if (current_time - recieve_previous_time >= recieve_time_trigger)
  {
    readcheck = recieve_data();
    if (readcheck < 0)
    {
      bt_error(readcheck);
      inboard_error(readcheck);
    }
    else
    {
      state_check(readcheck);
    }
    recieve_previous_time = current_time;
  }
  //package and send timer
  if (current_time - package_previous_time >= package_time_trigger)
  {
    if ((packagecheck = package_bundle()) >= 0)
    {
      Serial.println("----Successful Packaging----");
      if ((sendcheck = send_bundle()) > 0)
      {
        Serial.println("----Successful Sending----");
      } //sendcheck if
    }   //packagecheck if
    package_previous_time = current_time;
  } //package timer
} //eof
void flow_sensor_pulse_counter()
{
  flow_sensor_pulse_count++;
}
char *convert_float_to_string(float n, int stringlength, int precision)
{
  char buff[stringlength];
  char* float_to_be_converted = buff;
  dtostrf(n, stringlength, precision, float_to_be_converted);
  return float_to_be_converted;
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
  if ((millis() - old_flow_time > 1000))
  {
    detachInterrupt(sensor_interrupt);
    flow_rate_ml = ((1000.0 / (millis() - old_flow_time)) * flow_sensor_pulse_count) / flow_sensor_tick_rate;
    old_flow_time = millis();
    flow_volume_ml = (flow_rate_ml / 60) * 1000;
    flow_total_ml += flow_volume_ml;
    Serial.print("Flow Rate:");
    Serial.print(flow_rate_ml);
    Serial.print("mL/min\n");
    Serial.print("Output Liquid Quantity: ");
    Serial.print(flow_total_ml);
    Serial.print("mL");
    Serial.print("\t"); // Print tab space
    long totallitres = flow_total_ml / 1000;
    Serial.print(totallitres);
    Serial.print("L");
    Serial.print("\n");
    flow_sensor_pulse_count = 0;
    attachInterrupt(sensor_interrupt, flow_sensor_pulse_counter, FALLING);
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
  arr[0] = convert_float_to_string(get_humidity(), 3, 2);
  arr[1] = convert_float_to_string(get_temperature(), 3, 2);
  arr[2] = convert_float_to_string(flow_rate_ml, 3, 2);
  arr[3] = convert_float_to_string(flow_total_ml, 3, 2);
  arr[4] = convert_float_to_string(flow_volume_ml, 3, 2);
  int n = sprintf(sender,
                  "Humidity:%s#\nTemperature:%s#\nFlow_Frequency:%s#\nFlow_flow_total_ml:%s#\nFlow_Rate_ml:%s#\nHall_Effect_State:%d#\nLevel_Switch_State:%i#\nUV_State %d#\n",
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
  int btcheck=digitalRead(bt_state);
  if(btcheck==LOW){
    Serial.println("----No bluetooth connection aborting data transmission----");
    return 0;
    }
  if (bt.availableForWrite())
  {
    Serial.println("Sending String:");
    Serial.print(sender);
    bt.println(sender);
    return 1;
  }
  else
    return -15;
}
int recieve_data()
{
   int btcheck=digitalRead(bt_state);
  if(btcheck==LOW){
    Serial.println("----No bluetooth connection aborting data transcription----");
    return 0;
    }
  int num = receiver;
  if (bt.available())
  {
    receiver = bt.read();
    if (num != receiver)
    {
      Serial.println(receiver);
      return receiver;
    }
    else
    {
      return 0;
    }
  }
  
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
