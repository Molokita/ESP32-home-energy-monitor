#include "ESPHelper.h"
#include <Metro.h>
#include "EmonLib.h"
#include <driver/adc.h>

#define SAMPLE_COUNT 5     //Number of measurements
#define VOLT_CAL 492       //ZMPT101B voltage sensor calibration
#define CURRENT_CAL1 52.6  //SCT013 50A/1V current sensor calibration

#define ADC_BITS 12     //set ADC to fully utilize the 12-bit resolution of ESP32, instead of the 10-bit default for the ATMEGA-328
#define ADC_INPUT1 36
#define ADC_INPUT2 39
#define ONBOARD_LED 2

// Create instance No. 1 of the energy monitor lib
EnergyMonitor emon1;

// Arrays to hold the sample data
double volts1[SAMPLE_COUNT];
double amps1[SAMPLE_COUNT];
double realPower1[SAMPLE_COUNT];
double apparentPower1[SAMPLE_COUNT];
double powerFactor1[SAMPLE_COUNT];

double ampsInst;
double voltsInst;

// Counter to keep track of the current sample location
int counter = 0;

const char* voltTopic = "<yourTopic>/volt";
const char* ampTopic = "<yourTopic>/amp";
const char* appowerTopic = "<yourTopic>/appower";
const char* realpowerTopic = "<yourTopic>/realpower";
const char* powerfactorTopic = "<yourTopic>/powerfactor";

const char* hostnameStr = "ESP32-power-monitor";
const char* otaPass = "<your OTA password>";

netInfo homeNet = {  .mqttHost = "MQTT server IP",     //can be blank if not using MQTT
          .mqttUser = "your MQTT user",   //can be blank
          .mqttPass = "your MQTT password",   //can be blank
          .mqttPort = 1883,         //default port for MQTT is 1883 - only chance if needed.
          .ssid = "your SSID",
          .pass = "your wifi psswd"};

ESPHelper myESP(&homeNet);

Metro powerMetro = Metro(60000);  // Timer set to 1-minute intervals

void setup() {
  
  Serial.begin(115200);
  Serial.println("++ ESP32 Power Monitor Debugging ++");  //enable only while debugging

  ampsInst = 0;  //enable only while debugging
  voltsInst = 0;  //enable only while debugging

   // Setup the ADC channels
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
  analogReadResolution(ADC_BITS);
  
  pinMode(ADC_INPUT1, INPUT);
  pinMode(ADC_INPUT2, INPUT);
  pinMode(ONBOARD_LED,OUTPUT);

  emon1.voltage(ADC_INPUT2, VOLT_CAL, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(ADC_INPUT1, CURRENT_CAL1);
  
  myESP.OTA_enable();
  myESP.OTA_setPassword(otaPass);
  myESP.OTA_setHostnameWithVersion(hostnameStr);
  myESP.setHopping(false);
  myESP.begin();

}

void loop(){

 //reset the var that keeps track of the number of samples taken 
  //(loop back around to 0 on the array for our running total)
  if(counter >= SAMPLE_COUNT){
    counter = 0;
  }

  Serial.println("Taking measurement...");  //enable only while debugging

  //calculate the most recent readings
  emon1.calcVI(20,5000);

  //save the voltage, current, watts to the array for later averaging
  //ampsInst = emon1.Irms;  //DEBUG
  //voltsInst = emon1.Vrms;   //DEBUG
  //amps1[counter] = emon1.calcIrms(1480); //not using this for now
  amps1[counter] = emon1.Irms;
  volts1[counter] = emon1.Vrms;
  realPower1[counter] = emon1.realPower;
  apparentPower1[counter] = emon1.apparentPower;
  powerFactor1[counter] = emon1.powerFactor;

  counter++;
    
  //setup the vars to be averaged
  double voltAvg1 = 0;
  double ampAvg1 = 0;
  double realpowAvg1 = 0;
  double appowAvg1 = 0;
  double pfAvg1 = 0;

  //add em up for averaging
  for(int i = 0; i < SAMPLE_COUNT; i++){
    voltAvg1 += volts1[i];
    ampAvg1 += amps1[i];
    realpowAvg1 += realPower1[i];
    appowAvg1 += apparentPower1[i];
    pfAvg1 += powerFactor1[i];
}

  //get the final average by dividing by the # of samples
   ampAvg1 /= SAMPLE_COUNT;
   voltAvg1 /= SAMPLE_COUNT;
   realpowAvg1 /= SAMPLE_COUNT;
   appowAvg1 /= SAMPLE_COUNT;
   pfAvg1 /= SAMPLE_COUNT;
   
  Serial.println("Finished measurement!");

    //only send the data every so often (set by the metro timer) and only when connected to WiFi and MQTT
    if(myESP.loop() == FULL_CONNECTION && powerMetro.check()){
        
      Serial.println("Posting data...");

      digitalWrite(ONBOARD_LED, HIGH);

      // Post volts
      char voltStr[10];
      dtostrf(voltAvg1,4,1,voltStr);
      myESP.publish(voltTopic,voltStr, true);
      delay(5);

      // Post amps
      char ampStr[10];
      dtostrf(ampAvg1,4,1,ampStr);
      //dtostrf(ampsInst,4,1,ampStr);
      myESP.publish(ampTopic,ampStr, true); 
      delay(5);

      // Post real power
      char rpowStr[10];
      dtostrf(realpowAvg1,4,1,rpowStr);
      myESP.publish(realpowerTopic,rpowStr, true);
      delay(5);

      // Post apparent power
      char appowStr[10];
      dtostrf(appowAvg1,4,1,appowStr);
      myESP.publish(appowerTopic,appowStr, true);
      delay(5);

      // Post power factor
      char pfStr[10];
      dtostrf(pfAvg1,4,1,pfStr);
      myESP.publish(powerfactorTopic,pfStr, true);
      delay(5);

      Serial.println("Data Successfully posted!");

      digitalWrite(ONBOARD_LED, LOW);
 
    }

    yield();

}


void callback(char* topic, uint8_t* payload, unsigned int length) {

}
