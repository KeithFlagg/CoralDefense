#include <dht.h>
#define uv_pin 22
#define he_pin 23
#define dht_pin 4
#define fs_pin 2
#define ls_pin 20
#define led_pin 53
dht DHT;
int lsstate;
int x;
int y;
float tee = 0.0;
float frequency = 0;
float water = 0.0;
float total = 0.0;
float ls = 0.0;
int state;
void turnoffuv();
void turnonuv();
void levelswitch();
void flowsense();
void setup_humidsensor();
float get_humidity();
float get_temperature();
int get_he_state();
void setup()
{
  pinMode(led_pin, OUTPUT);
  pinMode(ls_pin, INPUT_PULLUP);
  pinMode(he_pin, INPUT);
  pinMode(uv_pin, OUTPUT);
  pinMode(fs_pin, INPUT);
  Serial.begin(9600);
}

void loop()
{
  flowsense();
  Serial.print("Humidity =");
  Serial.print(get_humidity());
  Serial.print("%     \n");
  Serial.print("Temperature =");
  Serial.print(get_temperature());
  Serial.print("c\n");

  state = get_he_state();
  if (state == LOW)
  {
    Serial.print("detected\n");
  }
  else
  {
    Serial.print("nothing detected\n");
  }
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
void flowsense()
{
  Serial.println("Flow sensor");
  x = pulseIn(fs_pin, HIGH);
  y = pulseIn(fs_pin, LOW);
  tee = (float)(x + y);
  frequency = 1000000 / tee;
  water = frequency / 7.5;
  ls = water / 60;
  if (frequency >= 0)
  {
    if (isinf(frequency))
    {
      Serial.println("Vol :0.0");
      Serial.print("Total: ");
      Serial.print(total);
    }
    else
    {
      total += ls;
      Serial.println(frequency);
      Serial.print("Vol.: ");
      Serial.print(water);
      Serial.print("L/M\n");
      Serial.print("Total: ");
      Serial.print(total);
    }
  }
  delay(1000);
}
void turnoffuv()
{
  Serial.println("Turning off UV light");
  digitalWrite(uv_pin, LOW);
  delay(1000);
}
void turnonuv()
{
  Serial.println("Turning on UV light");
  digitalWrite(uv_pin, HIGH);
  delay(1000);
}
void levelswitch()
{
  lsstate = digitalRead(ls_pin);
  if (lsstate == HIGH)
  {
    digitalWrite(led_pin, LOW);
    turnoffuv();
  }
  else
  {
    digitalWrite(led_pin, HIGH);
    turnonuv();
  }
}