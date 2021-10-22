/*
* pixelinterface.ino - Drive LEDs and combine multiple universes into a single string
*
* Project: ESP Lighting Controller
* Author: Andy Milluzzi, 08milluz.com
* 
* This application connects to a production network and listens for packets
* Once data is received, it then sends it to the correct pixel to display.
* If no data is recieved, it goes into a cross fade of colors.
*
*/
//#include <dmx.h> // DMX is commented out for now
#include <ESPAsyncE131.h>
#include <FastLED.h>
#include <Ticker.h>


 
#define UNIVERSE1 3101
#define UNIVERSE2 3102
#define UNIVERSE3 3103
#define UNIVERSE4 3104
#define UNIVERSE5 3105
#define UNIVERSE_COUNT 5                // Total number of Universes to listen for, starting at UNIVERSE
#define NUM_LEDS_PER_UNIVERSE 170
#define DATA_PIN_1 23
#define DATA_PIN_2 21
#define DATA_PIN_3 32
#define DATA_PIN_4 25
#define DATA_PIN_5 27

const char ssid[] = "SSID";         // Replace with your SSID
const char passphrase[] = "Password";   // Replace with your WPA2 passphrase

// Static IP address
IPAddress local_IP(192, 168, 25, 71);
// Gateway (Router) IP address
IPAddress gateway(192, 168, 25, 1);

IPAddress subnet(255, 255, 255, 0);

// Since the node wont be on the internet DNS doenst matter
// Using google's DNS servers just to have something
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

CRGB pixels1[NUM_LEDS_PER_UNIVERSE];
CRGB pixels2[NUM_LEDS_PER_UNIVERSE];
CRGB pixels3[NUM_LEDS_PER_UNIVERSE];
CRGB pixels4[NUM_LEDS_PER_UNIVERSE];
CRGB pixels5[NUM_LEDS_PER_UNIVERSE];


CRGB yellow = CRGB(255, 115, 0);
CRGB orange = CRGB(250, 25, 0);
CRGB purple = CRGB(150, 0, 230);
CRGB black = CRGB(0, 00, 0);

CRGBPalette16 currentPalette(black);
CRGBPalette16 targetPalette(orange);


int timeout = 0;
#define TIMEOUT 30
// ESPAsyncE131 instance with UNIVERSE_COUNT buffer slots
ESPAsyncE131 e131(UNIVERSE_COUNT);

Ticker fadeTimer;

bool fade = false;
bool triggerTimer = false;
unsigned int fadeCounter = 0;

void startFade()
{
  Serial.println("Triggered fade");
  for(int i = 0; i < 255; i++)
  {
    fadeToBlackBy(pixels1, 170, 1);
    fadeToBlackBy(pixels2, 170, 1);
    fadeToBlackBy(pixels3, 170, 1);
    fadeToBlackBy(pixels4, 170, 1);
    fadeToBlackBy(pixels5, 170, 1);
    FastLED.show();
    delay(20);
  }
  delay(1000);

  fade = true;
  fadeCounter = 0;
  currentPalette = CRGBPalette16(black);
  targetPalette = CRGBPalette16(orange);
}

void setup() {
    Serial.begin(115200);
    delay(10);
    //DMX::Initialize(output);
    Serial.println("Initializing pixels");
    FastLED.addLeds<WS2812B, DATA_PIN_1, GRB>(pixels1, NUM_LEDS_PER_UNIVERSE);
    FastLED.addLeds<WS2812B, DATA_PIN_2, GRB>(pixels2, NUM_LEDS_PER_UNIVERSE);
    FastLED.addLeds<WS2812B, DATA_PIN_3, GRB>(pixels2, NUM_LEDS_PER_UNIVERSE);
    FastLED.addLeds<WS2812B, DATA_PIN_4, GRB>(pixels2, NUM_LEDS_PER_UNIVERSE);
    FastLED.addLeds<WS2812B, DATA_PIN_5, GRB>(pixels2, NUM_LEDS_PER_UNIVERSE);

    for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
    {
      pixels1[i] = CRGB(255, 30, 0);
      pixels2[i] = CRGB(255, 20, 0);
      pixels3[i] = CRGB(255, 30, 0);
      pixels4[i] = CRGB(255, 20, 0);
      pixels5[i] = CRGB(255, 30, 0);
    }

    FastLED.show();
    
    
    // Make sure you're in station mode    
    WiFi.mode(WIFI_STA);

    // Configures static IP address
    //if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    //  Serial.println("STA Failed to configure");
    //}


    
    Serial.println("");
    Serial.print(F("Connecting to "));
    Serial.print(ssid);
    
    if (passphrase != NULL)
        WiFi.begin(ssid, passphrase);
    else
        WiFi.begin(ssid);

    while (WiFi.status() != WL_CONNECTED && timeout < TIMEOUT) {
        delay(500);
        Serial.print(".");
        timeout++;
    }

    if(timeout < TIMEOUT)
    {
      Serial.println("");
      Serial.print(F("Connected with IP: "));
      Serial.println(WiFi.localIP());
  
      Serial.println("Multicast");
      // Setup for multicast initially, but unicast might be better if your router struggles to keep up
      //if (e131.begin(E131_UNICAST))                              // Listen via Unicast
      if (e131.begin(E131_MULTICAST, UNIVERSE1, UNIVERSE_COUNT))   // Listen via Multicast
      {
          Serial.println(F("Listening for data..."));
          delay(1);
      }
      else 
      {
          Serial.println(F("*** e131.begin failed ***"));
  
      }
    }
    else
    {
      Serial.println("No Wifi");
    }

}

void loop() {
    if (!e131.isEmpty()) {
        fade = false;
        triggerTimer = false;
        fadeTimer.detach();
        e131_packet_t packet;
        e131.pull(&packet);     // Pull packet from ring buffer
        
       /* Serial.printf("Universe %u / %u Channels | Packet#: %u / Errors: %u / CH1: %u\n",
                htons(packet.universe),                 // The Universe for this packet
                htons(packet.property_value_count) - 1, // Start code is ignored, we're interested in dimmer data
                e131.stats.num_packets,                 // Packet counter
                e131.stats.packet_errors,               // Packet error counter
                packet.property_values[1]);             // Dimmer data for Channel 1

       */
       const unsigned int universe = htons(packet.universe);
       if(universe == UNIVERSE1)
       {
        for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
        {
          const int j = i * 3 + 1;
          pixels1[i] = CRGB(packet.property_values[j], packet.property_values[j+1], packet.property_values[j+2]);
          
        }
       }
       if(universe == UNIVERSE2)
       {
        //Serial.println("Processing universe 2");
        for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
        {
          const int j = i * 3 + 1;
          pixels2[i] = CRGB(packet.property_values[j], packet.property_values[j+1], packet.property_values[j+2]);
          //DMX::Write(i,packet.property_values[i]); 
        }
       }
       if(universe == UNIVERSE3)
       {
        //Serial.println("Processing universe 2");
        for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
        {
          const int j = i * 3 + 1;
          pixels3[i] = CRGB(packet.property_values[j], packet.property_values[j+1], packet.property_values[j+2]);
          //DMX::Write(i,packet.property_values[i]); 
        }
       }
       if(universe == UNIVERSE4)
       {
        //Serial.println("Processing universe 2");
        for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
        {
          const int j = i * 3 + 1;
          pixels4[i] = CRGB(packet.property_values[j], packet.property_values[j+1], packet.property_values[j+2]);
          //DMX::Write(i,packet.property_values[i]); 
        }
       }
       if(universe == UNIVERSE5)
       {
        //Serial.println("Processing universe 2");
        for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
        {
          const int j = i * 3 + 1;
          pixels5[i] = CRGB(packet.property_values[j], packet.property_values[j+1], packet.property_values[j+2]);
          //DMX::Write(i,packet.property_values[i]); 
        }
       }
       FastLED.show();
    }
    else
    {
      if(!triggerTimer)
      {
        triggerTimer = true;
        fadeTimer.once(30, startFade);
      }
      
    }

    if(fade)
    {
       const uint8_t maxChanges = 30; 
       nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);
       for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
        {
          pixels1[i] = ColorFromPalette( currentPalette, 0, 255);
          pixels2[i] = ColorFromPalette( currentPalette, 0, 255);
          pixels3[i] = ColorFromPalette( currentPalette, 0, 255);
          pixels4[i] = ColorFromPalette( currentPalette, 0, 255);
          pixels5[i] = ColorFromPalette( currentPalette, 0, 255);
        }

      FastLED.show();
      fadeCounter++;
      FastLED.delay(10);

      if(fadeCounter == 300)
      {
        currentPalette = targetPalette;
        targetPalette = CRGBPalette16(orange);
      }
      if(fadeCounter == 600)
      {

        currentPalette = targetPalette;
        targetPalette = CRGBPalette16(yellow);
      }
      if(fadeCounter == 900)
      {
        currentPalette = targetPalette;
        targetPalette = CRGBPalette16(purple);
        fadeCounter = 0;
      }
      
    }

}
