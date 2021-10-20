/*
* pixelinterfacetrees.ino - Drive LEDs and combine multiple universes into a single string
*
* Project: ESP Lighting Controller
* Author: Andy Milluzzi, 08milluz.com
*
*/
//#include <dmx.h>
#include <ESPAsyncE131.h>
#include <FastLED.h>
#include <Ticker.h>


 
#define UNIVERSE1 3001
#define UNIVERSE2   3002
#define UNIVERSE3   3003
#define UNIVERSE_COUNT 3                // Total number of Universes to listen for, starting at UNIVERSE
#define NUM_LEDS_PER_UNIVERSE 170
#define DATA_PIN_1 15
#define DATA_PIN_2 0

const char ssid[] = "SSID";         // Replace with your SSID
const char passphrase[] = "Password";   // Replace with your WPA2 passphrase

CRGB pixels1[NUM_LEDS_PER_UNIVERSE*2];
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
    fadeToBlackBy(pixels1, NUM_LEDS_PER_UNIVERSE*2, 1);
    fadeToBlackBy(pixels2, NUM_LEDS_PER_UNIVERSE, 1);
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
    FastLED.addLeds<WS2811, DATA_PIN_1, RGB>(pixels1, NUM_LEDS_PER_UNIVERSE*2
    );
    FastLED.addLeds<WS2811, DATA_PIN_2, RGB>(pixels2, NUM_LEDS_PER_UNIVERSE);

    for(int i = 0; i < NUM_LEDS_PER_UNIVERSE*2; i++)
    {
      pixels1[i] = CRGB(150, 0, 210);
    }
    for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
    {
      pixels2[i] = CRGB(150, 0, 210);
    }

    FastLED.show();
    
    
    // Make sure you're in station mode    
    WiFi.mode(WIFI_STA);
    
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
     //Choose one to begin listening for E1.31 data
    //if (e131.begin(E131_UNICAST))                               // Listen via Unicast
    if (e131.begin(E131_MULTICAST, UNIVERSE1, UNIVERSE_COUNT))   // Listen via Multicast
        Serial.println(F("Listening for data..."));
        //delay(1);
    else 
        Serial.println(F("*** e131.begin failed ***"));

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
          pixels1[i+NUM_LEDS_PER_UNIVERSE] = CRGB(packet.property_values[j], packet.property_values[j+1], packet.property_values[j+2]);
          //DMX::Write(i,packet.property_values[i]); 
        }
       }
       if(universe == UNIVERSE3)
       {
        //Serial.println("Processing universe 3");
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
       for(int i = 0; i < NUM_LEDS_PER_UNIVERSE*2; i++)
       {
         pixels1[i] = ColorFromPalette( currentPalette, 0, 255);
       }
       for(int i = 0; i < NUM_LEDS_PER_UNIVERSE; i++)
       {
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
