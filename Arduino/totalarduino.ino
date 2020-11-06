#include <dht.h> //Library needed for humidity and temperature sensors
#define uv_pin 22
#define he_pin 23
#define dht_pin 4
#define fs_pin 2
#define ls_pin 20
#define led_pin 53
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
int get_flow_info();//get all the information regarding the current flow 
int package_bundle();// gather data for and formate the bublde string return negative if error
int send_bundle();//send the bundle over the bt connection return negative if error
int recieve_bundle();//read the bundle from the bt connection into the reader, return negative if error
int get_he_state();//get hall effect state and return
int get_ls_state();//get level switch state and return
float get_humidity();//read humidity and return
float get_temperature();//read temperature and return
void turn_off_uv();//turns off the uv light
void turn_on_uv();//turns on the uv light
char *sender;// string to be formatted and sent over bluetooth
String *reader;//string to read the bluetooth
flowinfo *curflow;//current flow information
bundle *receiver;//bundle that gets read from the phone
bundle *transmitter;//bundle that sends to the phone
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
    Serial1.println("Sending error");
  }
  delay(500);
  readcheck = recieve_bundle();
  if (sendcheck < 0)
  {
    Serial.println("Reading error");
    Serial1.println("Reading error");
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

void turnoffuv()
{
  digitalWrite(uv_pin, LOW);
  delay(1000);
}
void turnonuv()
{
  digitalWrite(uv_pin, HIGH);
  delay(1000);
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
  int n = sprintf(sender, "Humidity: %.2f Temperature: %.2f Flow_Frequency: %.2f Flow_Total: %.2f Flow_Volume: %.2f Hall_Effect_State: %i Level_Switch_State: %i UV_State: %i #",
                  transmitter->humidity, transmitter->temperature, transmitter->flow_frequency, transmitter->flow_total, transmitter->flow_volume, transmitter->hall_effect_state, transmitter->level_switch_state, transmitter->UV_state);
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
int recieve_bundle()
{
  if (bt.available())
  {
    *reader = bt.readStringUntil('#');
    return 0;
  }
  else
    return -12;
}
