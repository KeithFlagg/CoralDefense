//level switch high signal, shut down fresh water pump and turn of uv bar
//level switch low turn on uv sterilization and fresh water pump
#include <dht.h> //Library needed for humidity and temperature sensors
#define uv_pin 21
#define he_pin 23
#define dht_pin 4
#define fs_pin 2
#define ls_pin 20
#define led_pin 53
#define fwfp_pin 22
//defining device pins
#define bt Serial1
//just makes it easier to code
//all the information relating to the flow sensors
struct flowinfo
{
  float frequency;
  float volume;
  float total;
};
//information that will be sent over and read from bluetooth
struct bundle
{
  float humidity;
  float temperature;
  float flow_frequency;
  float flow_volume;
  float flow_total;
  int hall_effect_state;
  int level_switch_state;
  int UV_state;
};
//make a dht sensor object
dht DHT;
int bt_error(int);
int inboard_error(int);
int get_flow_info();            //get all the information regarding the current flow
int package_bundle();           // gather data for and formate the bublde string return negative if error
int send_bundle();              //send the bundle over the bt connection return negative if error
int recieve_data();             //read the bundle from the bt connection into the reader, return negative if error
int get_he_state();             //get hall effect state and return
int get_ls_state();             //get level switch state and return
void turn_off_UV();             //turns off the uv light does not return a value because the call to the pin is void
void turn_on_UV();              //turns on the uv light does not return a value because the call to the pin is void
int state_check();              //state handler for recieved data
float get_humidity();           //read humidity and return
float get_temperature();        //read temperature and return
void turn_on_fwf_pump();        //turns on the fresh water fill pump
void turn_off_fwf_pump();       //turns off the freshwater fill pump
char *sender;                   // string to be formatted and sent over bluetooth
flowinfo *curflow;              //current flow information
int receiver;                   //int that gets read from the phone
bool change_from_phone = false; //false if no change true if change
bundle *transmitter;            //bundle that sends to the phone
void setup()
{
  pinMode(led_pin, OUTPUT);
  pinMode(ls_pin, INPUT_PULLUP);
  pinMode(he_pin, INPUT);
  pinMode(uv_pin, OUTPUT);
  pinMode(fs_pin, INPUT);
  Serial.begin(9600);
  bt.begin(9600);
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
  delay(500);
  readcheck = recieve_data();
  if (readcheck < 0)
  {
    Serial.println("Reading error");
    bt.println("Reading error");
  }
  if (change_from_phone == true)
  {
    int stater;
    if ((stater = state_check()) < 0)
    {
      inboard_error(stater);
    }
  }

  delay(500);
}
float get_humidity()
{
  DHT.read11(dht_pin);
  float humidity = DHT.humidity;
  return humidity;
}
float get_temperature()
{
  DHT.read11(dht_pin);
  float temperature = DHT.temperature;
  return temperature;
}
int get_he_state()
{
  return digitalRead(he_pin);
}
void turn_off_UV()
{
  digitalWrite(uv_pin, LOW);
}
void turn_on_UV()
{
  digitalWrite(uv_pin, HIGH);
}
int get_uv_state()
{
  int uvstate = digitalRead(uv_pin);
  return uvstate;
}
int get_ls_state()
{
  int lsstate = digitalRead(ls_pin);
  return lsstate;
}
int get_flow_info()
{

  int x = pulseIn(fs_pin, HIGH);
  int y = pulseIn(fs_pin, LOW);
  float tee = (float)(x + y);
  curflow->frequency = 1000000 / tee;
  curflow->volume = curflow->frequency / 7.5;
  float ls = curflow->volume / 60;
  if (curflow->frequency >= 0)
  {
    if (isinf(curflow->frequency))
    {
      curflow->total = 0;
      curflow->volume = 0;
      return 0;
    }
    else
    {
      curflow->total += ls;
      return 0;
    }
  }
}
int package_bundle()
{
  transmitter->flow_frequency = curflow->frequency;
  transmitter->flow_total = curflow->total;
  transmitter->flow_volume = curflow->volume;
  transmitter->hall_effect_state = get_he_state();
  transmitter->humidity = get_humidity();
  transmitter->level_switch_state = get_ls_state();
  transmitter->temperature = get_temperature();
  transmitter->UV_state = get_uv_state();
  int n = sprintf(sender,
                  "Humidity:%.2f#\nTemperature:%.2f#\nFlow_Frequency:%.2f#\nFlow_Total:%.2f#\nFlow_Volume:%.2f#\nHall_Effect_State:%i#\nLevel_Switch_State:%i#\nUV_State %i#\n",
                  transmitter->humidity, transmitter->temperature, transmitter->flow_frequency,
                  transmitter->flow_total, transmitter->flow_volume, transmitter->hall_effect_state, transmitter->level_switch_state, transmitter->UV_state);
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
      change_from_phone = true;
    }
    return 0;
  }
  else
    return -12;
}
int state_check()
{
  switch (receiver)
  {
  case 112:
    turn_on_UV();
    change_from_phone = false;
    return 20;
    break;
  case 113:
    turn_off_UV();
    change_from_phone = false;
    return 21;
    break;
  case 114:
    turn_on_fwf_pump(); //TODO
    change_from_phone = false;
    return 22;
    break;
  case 115:
    turn_off_fwf_pump(); //TODO
    change_from_phone = false;
    return 22;
    break;
  default:
    int n = -20;
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
