#include <Adafruit_NeoPixel.h>

#include <ESPSoftwareSerial.h>

#include "ESP8266WiFi.h"
#include <VariPass.h>

#include "Shared.h"
#include "TwiFi.h"

#include "Configuration.h"
#include "ConfigurationLED.h"
#include "ConfigurationSensors.h"
#include "ConfigurationVariPass.h"
#include "ConfigurationLuna.h"
#include "ConfigurationWiFi.h"


#define PIN_PIXEL       13
#define PIN_SERIAL_RX   12
#define PIN_SERIAL_TX   14

#define PULSE_DONE 0
#define PULSE_FAIL 1
#define PULSE_ERRO 2

#define URL_RESULT_DONE 0
#define URL_RESULT_FAIL 1


/* LEDs */
Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, PIN_PIXEL, NEO_GRB + NEO_KHZ800);
float intensity = 1.0;

struct LEDPower {
	RGB led {ledPowerColor.r, ledPowerColor.g, ledPowerColor.b};
} ledPower;

/* Misc. Values */
unsigned long timeUpload = 0;
unsigned long timeLight = 0;

/* Sensors */
SoftwareSerial softSerial(PIN_SERIAL_RX, PIN_SERIAL_TX, false, 256);

struct pms5003data {
    uint16_t framelen;
    uint16_t pm10_standard, pm25_standard, pm100_standard;
    uint16_t pm10_env, pm25_env, pm100_env;
    uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
} pms;

struct SensorParicles {
	uint16_t pm010[MAX_SAMPLE_COUNT] = {0};
	uint16_t pm025[MAX_SAMPLE_COUNT] = {0};
	uint16_t pm100[MAX_SAMPLE_COUNT] = {0};
	uint16_t raw003[MAX_SAMPLE_COUNT] = {0};
	uint16_t raw005[MAX_SAMPLE_COUNT] = {0};
	uint16_t raw010[MAX_SAMPLE_COUNT] = {0};
	uint16_t raw025[MAX_SAMPLE_COUNT] = {0};
	uint16_t raw050[MAX_SAMPLE_COUNT] = {0};
	uint16_t raw100[MAX_SAMPLE_COUNT] = {0};
	int count = 0;
} sensorParticles;




void setupLED();
void setupPreConnect();

void setupSensors();
void setupSensor_pms();

void processTicks();

bool processSensorParticlesSample();
void processSensorParticlesUpload();

void loadLightData();
bool readPMSdata(Stream *s);
void calculateIntensity(uint16_t light);
void ledPowerSet();
void ledNotifPulse(int pulse, RGB * color);
int openURL(String url);




void setupLED() {
    strip.begin();
}

void setupPreConnect() {
    uint16_t light = SENSOR_LIGHT_START;
	calculateIntensity(light);
	ledPowerSet();
}

void setupSensors() {
	setupSensor_pms();
}

void setupSensor_pms() {
    softSerial.begin(9600);
}




void processTicks() {
	if(processSensorParticlesSample()) {
		if (millis() - timeUpload >= MEASURE_DURATION * 1000) {
	        timeUpload = millis();
	        processSensorParticlesUpload();
	    }
		if (millis() - timeLight >= LIGHT_DURATION * 1000) {
	        timeLight = millis();
	        loadLightData();
	    }
	}
}

bool processSensorParticlesSample() {
	if (readPMSdata(&softSerial)) {
		if (SENSOR_PARTICLE_DEBUG) {
	        Serial.println("---------------------------------------");

	        Serial.print("PM1.0 value: ");
	        Serial.println(pms.pm10_standard);
	        Serial.print("PM2.5 value: ");
	        Serial.println(pms.pm25_standard);
	        Serial.print("PM10.0 value: ");
	        Serial.println(pms.pm100_standard);

	        Serial.print("Particles >  0.3um / 0.1L air: ");
	        Serial.println(pms.particles_03um);
	        Serial.print("Particles >  0.5um / 0.1L air: ");
	        Serial.println(pms.particles_05um);
	        Serial.print("Particles >  1.0um / 0.1L air: ");
	        Serial.println(pms.particles_10um);
	        Serial.print("Particles >  2.5um / 0.1L air: ");
	        Serial.println(pms.particles_25um);
	        Serial.print("Particles >  5.0um / 0.1L air: ");
	        Serial.println(pms.particles_50um);
	        Serial.print("Particles > 10.0um / 0.1L air: ");
	        Serial.println(pms.particles_100um);
	    }
	    sensorParticles.pm010[sensorParticles.count] = pms.pm10_standard;
	    sensorParticles.pm025[sensorParticles.count] = pms.pm25_standard;
	    sensorParticles.pm100[sensorParticles.count] = pms.pm100_standard;

	    sensorParticles.raw003[sensorParticles.count] = pms.particles_03um;
	    sensorParticles.raw005[sensorParticles.count] = pms.particles_05um;
	    sensorParticles.raw010[sensorParticles.count] = pms.particles_10um;
	    sensorParticles.raw025[sensorParticles.count] = pms.particles_25um;
	    sensorParticles.raw050[sensorParticles.count] = pms.particles_50um;
	    sensorParticles.raw100[sensorParticles.count] = pms.particles_100um;
	    sensorParticles.count++;
	    return true;
    }
    else {
    	return false;
    }
}

void processSensorParticlesUpload() {

	// Process
	float apm010 = 0;
	float apm025 = 0;
	float apm100 = 0;

	float avg003 = 0;
	float avg005 = 0;
	float avg010 = 0;
	float avg025 = 0;
	float avg050 = 0;
	float avg100 = 0;

	for (int i = 0; i < sensorParticles.count; i++)
		apm010 += (float) sensorParticles.pm010[i] / sensorParticles.count;
	for (int i = 0; i < sensorParticles.count; i++)
		apm025 += (float) sensorParticles.pm025[i] / sensorParticles.count;
	for (int i = 0; i < sensorParticles.count; i++)
		apm100 += (float) sensorParticles.pm100[i] / sensorParticles.count;

	for (int i = 0; i < sensorParticles.count; i++)
		avg003 += (float) sensorParticles.raw003[i] / sensorParticles.count;
	for (int i = 0; i < sensorParticles.count; i++)
		avg005 += (float) sensorParticles.raw005[i] / sensorParticles.count;
	for (int i = 0; i < sensorParticles.count; i++)
		avg010 += (float) sensorParticles.raw010[i] / sensorParticles.count;
	for (int i = 0; i < sensorParticles.count; i++)
		avg025 += (float) sensorParticles.raw025[i] / sensorParticles.count;
	for (int i = 0; i < sensorParticles.count; i++)
		avg050 += (float) sensorParticles.raw050[i] / sensorParticles.count;
	for (int i = 0; i < sensorParticles.count; i++)
		avg100 += (float) sensorParticles.raw100[i] / sensorParticles.count;

	avg003 -= avg005;
	avg005 -= avg010;
	avg010 -= avg025;
	avg025 -= avg050;
	avg050 -= avg100;

	// Upload
    int result;

    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PM_010, apm010, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_PM010);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_PM010);

    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PM_025, apm025, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_PM025);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_PM025);

    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PM_100, apm100, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_PM100);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_PM100);


    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PARTICLES_003, avg003 * SENSOR_PARTICLE_MULTI, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_Pr003);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_Pr003);

    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PARTICLES_005, avg005 * SENSOR_PARTICLE_MULTI, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_Pr005);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_Pr005);

    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PARTICLES_010, avg010 * SENSOR_PARTICLE_MULTI, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_Pr010);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_Pr010);

    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PARTICLES_025, avg025 * SENSOR_PARTICLE_MULTI, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_Pr025);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_Pr025);

    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PARTICLES_050, avg050 * SENSOR_PARTICLE_MULTI, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_Pr050);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_Pr050);

    varipassWriteFloat(VARIPASS_KEY, VARIPASS_ID_PARTICLES_100, avg100 * SENSOR_PARTICLE_MULTI, &result, 2);        
    if (result == VARIPASS_RESULT_SUCCESS)
        ledNotifPulse(PULSE_DONE, &ledSensorPMS_Pr100);
    else
        ledNotifPulse(PULSE_FAIL, &ledSensorPMS_Pr100);

    sensorParticles.count = 0;
}



void loadLightData() {
    int result;

    float light = varipassReadFloat(VARIPASS_KEY, VARIPASS_ID_LIGHT, &result);

	if (result == VARIPASS_RESULT_SUCCESS) {
		if (SENSOR_LIGHT_DEBUG)
    		Serial.println("Value successfully read! Value is: " + String(light));

    	if (SENSOR_LIGHT_LOG) {
    		uint16_t loglight = (uint16_t)lroundf(exp(light));
    		if (loglight - 1 > 0)
    			loglight--;
			if (SENSOR_LIGHT_DEBUG)
    			Serial.println("Restored from log value: " + String(loglight));
    		calculateIntensity(loglight);
    	}
    	else {
    		calculateIntensity((uint16_t)light);
    	}

		ledPowerSet();
    }
    else {
    	if (SENSOR_LIGHT_DEBUG)
	    	Serial.println("An error has occured reading the value! " + varipassGetResultDescription(result));
	}
}

bool readPMSdata(Stream *s) {
    if (!s->available()) {
        return false;
    }

    if (s->peek() != 0x42) {
        s->read();
        return false;
    }

    if (s->available() < 32) {
        return false; 
    } 
    
    uint8_t buf[32]; 
    uint16_t sum = 0; 
    s->readBytes(buf, 32);
    
    for (uint8_t i=0; i<30; i++) {
        sum += buf[i];
    }

    uint16_t buf_u16[15];
    for (uint8_t i=0; i<15; i++) {
        buf_u16[i] = buf[2 + i*2 + 1];
        buf_u16[i] += (buf[2 + i*2] << 8);
    }
    
    memcpy((void *)&pms, (void *)buf_u16, 30);
    if (sum != pms.checksum) {
		if (SENSOR_PARTICLE_DEBUG) {
        	Serial.println("Checksum failure");
        }
        return false;
    }

    return true;
}

void calculateIntensity(uint16_t light) {
	if (light < LED_LIGHT_MIN) {
    	if (LED_LOW_ENABLE)
        	intensity = -1;
    	else
        	intensity = (float) LED_BRIGHT_MIN / 255;
	}
    else if (light > LED_LIGHT_MAX)
        intensity = (float) LED_BRIGHT_MAX / 255;
    else
        intensity = (LED_BRIGHT_MIN + (LED_BRIGHT_MAX - LED_BRIGHT_MIN) * ((float) (light - LED_LIGHT_MIN) / (LED_LIGHT_MAX - LED_LIGHT_MIN))) / 255;
}

void ledPowerSet() {
	if (intensity == -1)
    	strip.setPixelColor(0, strip.Color(ledLowColor.r, ledLowColor.g, ledLowColor.b));
    else
    	strip.setPixelColor(0, strip.Color(ledPower.led.r * intensity, ledPower.led.g * intensity, ledPower.led.b * intensity));
    strip.show();
}

void ledNotifPulse(int pulse, RGB * color) {
	if (pulse == PULSE_DONE || pulse == PULSE_FAIL) {
        if (intensity == -1)
        	strip.setPixelColor(1, strip.Color(ledLowColor.r, ledLowColor.g, ledLowColor.b));
        else
	    	strip.setPixelColor(1, strip.Color(color->r * intensity, color->g * intensity, color->b * intensity));
	    strip.show();

		if (pulse == PULSE_DONE)
			delay(LED_NOTIF_PULSE_SEND_DONE);
		else if (pulse == PULSE_FAIL)
			delay(LED_NOTIF_PULSE_SEND_FAIL);

	    strip.setPixelColor(1, strip.Color(0, 0, 0)); 
	    strip.show();  
	}
	else if (pulse == PULSE_ERRO) {
		for (int i = 0; i < 4; i++) {
		    strip.setPixelColor(1, strip.Color(color->r * intensity, color->g * intensity, color->b * intensity));
		    strip.show();
		    delay(LED_NOTIF_PULSE_SENSOR_ERROR);
	    	strip.setPixelColor(1, strip.Color(0, 0, 0)); 
	    	strip.show();  
		    delay(LED_NOTIF_PULSE_SENSOR_ERROR);
		}
	}
}

int openURL(String url) {
    if (LUNA_DEBUG)
        Serial.println("Opening URL: " + String(LUNA_IP) + url);
        
    WiFiClient client;
    if (!client.connect(LUNA_IP, LUNA_PORT)) {  
        if (LUNA_DEBUG)
            Serial.println("Error connecting!");
        return URL_RESULT_FAIL;
    }

    client.print("GET " + url + " HTTP/1.1\r\n" +
                 "Host: " + LUNA_IP + "\r\n" + 
                 "Connection: close\r\n\r\n");
    client.stop();
    
    if (LUNA_DEBUG)
        Serial.println("Connection success.");

    return URL_RESULT_DONE;
}



void setup() {
    Serial.begin(115200);

    setupLED();
	setupSensors();
	setupPreConnect();
	connectWiFi(true);
	loadLightData();

	if (!LED_COLOR_DEBUG) {
		openURL(String(LUNA_URL_BOOT) + "&key=" + String(LUNA_KEY) + "&device=" + String(WIFI_HOST));
		delay(INITIAL_WAIT * 1000);
        ledNotifPulse(PULSE_DONE, &ledPowerColor);
	    timeUpload = millis();
	    timeLight = millis();
	}
}

void loop() {	
	if (!LED_COLOR_DEBUG) {
		processTicks();

		if (WiFi.status() != WL_CONNECTED) {
			connectWiFi(true);
	    }
	}
	else {
		strip.setPixelColor(1, strip.Color(ledDebug.r * intensity, ledDebug.g * intensity, ledDebug.b * intensity)); 
		strip.show();
	} 
}

