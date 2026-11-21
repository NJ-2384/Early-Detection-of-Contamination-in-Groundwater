#define phPin A3      //pH meter Analog output to Arduino Analog Input 0
float calibration_value = 21.34 + 0.7;
unsigned long int avgValue;  //Store the average value of the sensor feedback
int buf[10], temp;
void setup(){
pinMode(phPin, INPUT);
Serial.begin(115200);
}
void loop(){
  
  avgValue=analogRead(phPin);
  //take the average value of 6 center sample
    
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue = (-5.70 * phValue + calibration_value)/2; //convert the millivolt into pH value
  
  Serial.println(phValue); 

  delay(1000);
}