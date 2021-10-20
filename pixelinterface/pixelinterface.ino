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


 
#define UNIVERSE1 2001
#define UNIVERSE2   2002
#define UNIVERSE_COUNT 2                // Total number of Universes to listen for, starting at UNIVERSE
#define NUM_LEDS_PER_UNIVERSE 170
#define DATA_PIN_1 15
#define DATA_PIN_2 0

const char ssid[] = "SSID";         // Replace with your SSID
const char passphrase[] = "Password";   // Replace with your WPA2 passphrase

// Static IP address
IPAddress local_IP(192, 168, 1, 50);
// Gateway (Router) IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);

// Since the node wont be on the internet DNS doenst matter
// Using google's DNS servers just to have something
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

CRGB pixels1[NUM_LEDS_PER_UNIVERSE];
CRGB pixels2[NUM_LEDS_PER_UNIVERSE];


CRGB purple = CRGB(150, 0, 210);
CRGB orange = CRGB(255, 25, 0);
CRGB green = CRGB(0, 200, 0);
CRGB black = CRGB(0, 00, 0);

CRGBPalette16 currentPalette(black);
CRGBPalette16 targetPalette(purple);



// ESPAsyncE131 instance with UNIVERSE_COUNT buffer slots
ESPAsyncE131 e131(UNIVERSE_COUNT);

Ticker fadeTimer;

bool fade = false;
bool triggerTimer = false;
unsigned int fadeCounter = 0;

void startFade()
{
  //Serial.println("Triggered fade");
  for(int i = 0; i < 255; i++)
  {
    fadeToBlackBy(pixels1, 170, 1);
    fadeToBlackBy(pixels2, 170, 1);
    FastLED.show();
    delay(20);
  }
  delay(1000);

  fade = true;
  fadeCounter = 0;
  currentPalette = CRGBPalette16(black);
  targetPalette = CRGBPalette16(purple);
}

void setup() {
    Serial.begin(115200);
    delay(10);
    //DMX::Initialize(output);
    Serial.println("Initializing pixels");
    FastLED.addLeds<GS1903, DATA_PIN_1, BRG>(pixels1, NUM_LEDS_PER_UNIVERSE);
    FastLED.addLeds<GS1903, DATA_PIN_2, BRG>(pixels2, NUM_LEDS_PER_UNIVERSE);

    for(int i = 2; i < NUM_LEDS_PER_UNIVERSE; i++)
    {
      pixels1[i] = CRGB(150, 0, 210);
      pixels2[i] = CRGB(150, 0, 210);
    }

    FastLED.show();
    
    
    // Make sure you're in station mode    
    WiFi.mode(WIFI_STA);

    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }


    
    Serial.println("");
    Serial.print(F("Connecting to "));
    Serial.print(ssid);
    
    if (passphrase != NULL)
        WiFi.begin(ssid, passphrase);
    else
        WiFi.begin(ssid);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }


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
        while(1);
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
       const uint8_t maxChanges = 25; 
       nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);
       for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
        {
          pixels1[i] = ColorFromPalette( currentPalette, 0, 255);
          pixels2[i] = ColorFromPalette( currentPalette, 0, 255);
        }

      FastLED.show();
      fadeCounter++;
      FastLED.delay(10);

      if(fadeCounter == 1500)
      {
        currentPalette = targetPalette;
        targetPalette = CRGBPalette16(orange);
      }
      if(fadeCounter == 3000)
      {
        currentPalette = targetPalette;
        targetPalette = CRGBPalette16(green);
      }
      if(fadeCounter == 4500)
      {
        currentPalette = targetPalette;
        targetPalette = CRGBPalette16(purple);
        fadeCounter = 0;
      }
      
    }

}
