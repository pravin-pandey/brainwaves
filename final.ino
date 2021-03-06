#include <arduinoFFT.h>
#include <ESP8266WiFi.h>

const char* ssid  = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* host = "your_webiste_name_without_https:// or http://"; //"yaha aayega website ka naam"

#define SAMPLES 8             //Must be a power of 2
#define SAMPLING_FREQUENCY 768 //Hz, must be less than 10000 due to ADC

arduinoFFT FFT = arduinoFFT();

unsigned int sampling_period_us;
unsigned long microseconds;
 
double vReal[SAMPLES];
double vImag[SAMPLES];

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.print("Connecting to:");
  Serial.print(ssid);
  Serial.println();

  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED){
  delay(500);
  Serial.print(".");
  }
  Serial.println("\n WiFi connected");
  Serial.println("IP address:");
  Serial.print(WiFi.localIP());


  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
}

void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = analogRead(A0);
        vImag[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
 
    /*PRINT RESULTS*/   
    Serial.println("Frequency Value:");
    Serial.print(peak);
    Serial.println(" ");
     
    int device_id = 1;
    float eeg_freq = peak;
    if(isnan(eeg_freq))
    {
      Serial.println("Failed to read data");
      return;
    }
    Serial.println("connecting to: ");
    Serial.print(host);
 
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort))
    {
      Serial.println("connection failed");
      return;
    }
  
    String url = "/filepath/filename.php?device_id=" + String(device_id) + "&eeg_freq="+ String(eeg_freq);
    Serial.print("Requesting URL: ");
    Serial.println(url);
  
   client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
    delay(500);
  
    while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  delay(180000);
  Serial.println();
}
