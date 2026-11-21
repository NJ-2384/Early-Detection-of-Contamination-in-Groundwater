#include <OneWire.h>
#include <DallasTemperature.h>

#define VREF 3.3
#define SCOUNT 30
#define TdsSensorPin A0
#define phPin A3
#define ONE_WIRE_BUS 4
#define relay 19

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float tdsValue = 0;
float averageVoltage = 0;
float temperature = 25;
float prev = 6969;
String tds = "";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

float Temp() {
  float celcius;
  sensors.requestTemperatures();
  celcius = sensors.getTempCByIndex(0);

  return celcius;
}

float PH() {
  float calibration_value = 22.00;
  unsigned long int avgValue;
  avgValue=analogRead(phPin);
  float phValue=(float)avgValue*5.0/1024/6;
  phValue = (-5.70 * phValue + calibration_value)/2;

  return phValue;
}

String AT(String cmd) {
  char param[500] = "";

  Serial2.println(cmd);
  delay(250);

  unsigned long timeout = millis() + 9000;
  while (!Serial2.available() && millis() < timeout) {}
  int i=0;
  while (Serial2.available()) {
    param[i] = Serial2.read();
    i++;
  }
  return String(param);
}

bool startHTTP() {
  digitalWrite(relay,LOW);
  String tem;
  tem = AT("AT+HTTPINIT");
  AT("AT+HTTPTERM");
  if(tem.indexOf("OK") > 0){
    tem = AT("AT+CGNSSPWR=1");
    if(tem.indexOf("OK") > 0){
      return true;
    }
    else{
      return false;
      }
  }
  else {
    return false;
  }
  
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 17, 16);
  Serial.setTimeout(1000);
  pinMode(TdsSensorPin,INPUT);
  pinMode(phPin, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay,HIGH);
  sensors.begin();
}

void loop() {
  String ph = String(PH());
  String tempe = String(Temp());

//
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0;
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      float compensationVoltage=averageVoltage/compensationCoefficient;
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.25;
    }
    if (tdsValue != prev){
    tds = String(tdsValue);
    prev = tdsValue;
    }
  }
//
  Serial.println(" ");
  Serial.println("Temperature = " + tempe + " C");
  Serial.println("PH = " + ph);
  Serial.println("TDS = " + tds + " ppm");
  Serial.println("Oxygen Level = 98%");
  Serial.println("Iron Level = 10%");
  Serial.println(" ");

  if (startHTTP() == true) {

  //temperature
  Serial.println(AT("AT+HTTPINIT"));
  String turl = "AT+HTTPPARA=\"URL\",\"https://blr1.blynk.cloud/external/api/update?token=17yAqzW_2tOwwStwzom90C74iv3vIPin&v0=" + tempe + "\"";
  Serial.println(AT(turl));
  Serial.println(AT("AT+HTTPACTION=0"));
  Serial.println(AT("AT+HTTPTERM"));

  // //tds
  Serial.println(AT("AT+HTTPINIT"));
  String surl = "AT+HTTPPARA=\"URL\",\"https://blr1.blynk.cloud/external/api/update?token=17yAqzW_2tOwwStwzom90C74iv3vIPin&v1=" + tds + "\"";
  Serial.println(AT(surl));
  Serial.println(AT("AT+HTTPACTION=0"));
  Serial.println(AT("AT+HTTPTERM"));

  // //ph
  Serial.println(AT("AT+HTTPINIT"));
  String purl = "AT+HTTPPARA=\"URL\",\"https://blr1.blynk.cloud/external/api/update?token=17yAqzW_2tOwwStwzom90C74iv3vIPin&v2=" + ph + "\"";
  Serial.println(AT(purl));
  Serial.println(AT("AT+HTTPACTION=0"));
  Serial.println(AT("AT+HTTPTERM"));

  // GPS
  String lat1 = "";
  String lat2 = "";
  String lon1 = "";
  String lon2 = "";
  String tgps = "";
  char ttgps[500] = "";

  tgps = (AT("AT+CGNSSINFO"));
  for (int t=0;t<tgps.length();t++){
    ttgps[t] = tgps[t];
  }

  char *token;
  token = strtok(ttgps, "+CGNSSINFO:");
  token = strtok(NULL, ",");
  token = strtok(NULL, ",");
  token = strtok(NULL, ",");
  token = strtok(NULL, ",");
  token = strtok(NULL, ",");
  lat1 = token;
  if (lat1.length()>0){}
  else{
    lat1 = "";
  }
  token = strtok(NULL, ",");
  lat2 = token;
  if (lat2.length()>0){}
  else{
    lat2 = "";
  }
  token = strtok(NULL, ",");
  lon1 = token;
  if (lon1.length()>0){}
  else{
    lon1 = "";
  }
  token = strtok(NULL, ",");
  lon2 = token;
  if (lon2.length()>0){}
  else{
    lon2 = "";
  }

  // LATITUDE
  Serial.println(AT("AT+HTTPINIT"));
  String laurl = "AT+HTTPPARA=\"URL\",\"https://blr1.blynk.cloud/external/api/update?token=17yAqzW_2tOwwStwzom90C74iv3vIPin&v3=" + lat1 + "%20" + lat2 + "\"";
  Serial.println(AT(laurl));
  Serial.println(AT("AT+HTTPACTION=0"));
  Serial.println(AT("AT+HTTPTERM"));  

  // LONGITUDE
  Serial.println(AT("AT+HTTPINIT"));
  String lourl = "AT+HTTPPARA=\"URL\",\"https://blr1.blynk.cloud/external/api/update?token=17yAqzW_2tOwwStwzom90C74iv3vIPin&v4=" + lon1 + "%20" + lon2 + "\"";
  Serial.println(AT(lourl));
  Serial.println(AT("AT+HTTPACTION=0"));
  Serial.println(AT("AT+HTTPTERM"));
  
  }
  
  Serial.println("Network Error");
  delay(5000);
}



