/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-01-2025
 * @Date updated    04-01-2025 (By: Sebastiaan den Hertog)
 * @Description     Main controller for the Sphere LED system
 */

#ifndef ESPCONTROLLER_H
#define ESPCONTROLLER_H

// GLOBAL VARIABLES
// both declared and defined in header (solution from http://www.keil.com/support/docs/1868.htm)
//
// e.g. byte test = 2 becomes EM_GLOBAL byte test _INIT(2);
//     int arr[]{0,1,2} becomes EM_GLOBAL int arr[] _INIT_N(({0,1,2}));

#ifndef SPHERE_DEFINE_GLOBAL_VARS
#define EM_GLOBAL extern
#define _INIT(x)
#define _INIT_N(x)
#else
#define EM_GLOBAL
#define _INIT(x) = x

// Needed to ignore commas in array definitions
#define UNPACK(...) __VA_ARGS__
#define _INIT_N(x) UNPACK x
#endif

#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)

// Libraries
#include <WiFi.h>
#include <ETH.h>
#include <Preferences.h>
#include "const.h"
#include "internet.h"
#include "webServer.h"
#include <Littlefs.h>

EM_GLOBAL bool doInitBusses _INIT(false);

EM_GLOBAL bool useGlobalLedBuffer _INIT(true); // double buffering enabled on ESP32
EM_GLOBAL bool autoSegments _INIT(false);

// global ArduinoJson buffer
#define JSON_BUFFER_SIZE 24576

EM_GLOBAL StaticJsonDocument<JSON_BUFFER_SIZE> doc;
EM_GLOBAL volatile uint8_t jsonBufferLock _INIT(0);
EM_GLOBAL JsonDocument *fileDoc;
EM_GLOBAL bool doCloseFile _INIT(false);
EM_GLOBAL size_t fsBytesUsed _INIT(0);
EM_GLOBAL size_t fsBytesTotal _INIT(0);
EM_GLOBAL bool doSerializeConfig _INIT(false); // flag to initiate saving of config

// Sync Config
EM_GLOBAL int arlsOffset _INIT(0); // realtime LED offset

EM_GLOBAL IPAddress realtimeIP _INIT_N(((0, 0, 0, 0)));

// Network CONFIG
EM_GLOBAL IPAddress staticIP _INIT_N(((0, 0, 0, 0))); // static IP of ESP

// DMX Config

EM_GLOBAL uint16_t e131ProxyUniverse _INIT(0);
EM_GLOBAL uint16_t e131Universe _INIT(0);                       // settings for E1.31 (sACN) protocol (only DMX_MODE_MULTIPLE_* can span over consecutive universes)
EM_GLOBAL uint16_t e131Port _INIT(6454);                        // DMX in port. E1.31 default is 5568, Art-Net is 6454
EM_GLOBAL byte e131Priority _INIT(0);                           // E1.31 port priority (if != 0 priority handling is active)
EM_GLOBAL E131Priority highPriority _INIT(3);                   // E1.31 highest priority tracking, init = timeout in seconds
EM_GLOBAL byte DMXMode _INIT(DMX_MODE_MULTIPLE_RGB);            // DMX mode (s.a.)
EM_GLOBAL uint16_t DMXAddress _INIT(1);                         // DMX start address of fixture, a.k.a. first Channel [for E1.31 (sACN) protocol]
EM_GLOBAL uint16_t pollReplyCount _INIT(0);                     // count number of replies for ArtPoll node report
EM_GLOBAL byte e131LastSequenceNumber[E131_MAX_UNIVERSE_COUNT]; // to detect packet loss
EM_GLOBAL bool e131Multicast _INIT(false);                      // multicast or unicast
EM_GLOBAL bool e131SkipOutOfSequence _INIT(false);              // freeze instead of flickering

EM_GLOBAL byte DMXChannels _INIT(3); // number of channels per fixture
EM_GLOBAL byte DMXFixtureMap[15] _INIT_N(({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
// assigns the different channels to different functions. See wled21_dmx.ino for more information.
EM_GLOBAL uint16_t DMXGap _INIT(10);     // gap between the fixtures. makes addressing easier because you don't have to memorize odd numbers when climbing up onto a rig.
EM_GLOBAL uint16_t DMXStart _INIT(10);   // start address of the first fixture
EM_GLOBAL uint16_t DMXStartLED _INIT(0); // LED from which DMX fixtures start

// color mangling macros
#define RGBW32(r, g, b, w) (uint32_t((byte(w) << 24) | (byte(r) << 16) | (byte(g) << 8) | (byte(b))))
#define R(c) (byte((c) >> 16))
#define G(c) (byte((c) >> 8))
#define B(c) (byte(c))
#define W(c) (byte((c) >> 24))

class SphereController
{
public:
    SphereController();
    static SphereController &instance()
    {
        static SphereController instance;
        return instance;
    }

    // boot starts here
    void setup();

    void loop();
    void initLedSettings();
    void reset();

    void beginStrip();
    void initBusses();
};

#endif // SPHERECONTROLLER_H
