/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-01-2025
 * @Date updated    04-01-2025 (By: Sebastiaan den Hertog)
 * @Description     Multi Channel LED controller for the digital WS28** LED strip with DMX integration
 */

#define SPHERE_DEFINE_GLOBAL_VARS
#include "espController.h"
#include <Arduino.h>

Internet internet;
WebConfigServer webServer;

SphereController::SphereController() = default;

void SphereController::setup()
{
    Serial.begin(115200);
    delay(2500);
    DEBUG_PRINTLN("Starting Sphere Controller");

    DEBUG_PRINTLN("Starting internet");
    internet.begin();
    webServer.begin();

    initLedSettings();

    DEBUG_PRINTLN(F("Initializing strip"));
    beginStrip();
    DEBUG_PRINT(F("heap "));
    DEBUG_PRINTLN(ESP.getFreeHeap());

    delay(1000);

    if (e131.begin(e131Multicast, e131Port, e131Universe, E131_MAX_UNIVERSE_COUNT))
    {
        DEBUG_PRINTLN("E1.31 setup complete.");
    }
    else
    {
        DEBUG_PRINTLN("E1.31 setup failed.");
    }

    doInitBusses = true;

    delay(300);
    DEBUG_PRINTLN("ArtNet setup complete.");
}

void SphereController::loop()
{

    if (doInitBusses)
    {
        doInitBusses = false;
        initBusses();
        doSerializeConfig = true; // Save Config
    }
    if (doSerializeConfig)
        serializeConfig();

    if (e131NewData && millis() - strip.getLastShow() > 15)
    {
        e131NewData = false;
        DEBUG_PRINTF("TTS: %d\n", millis() - strip.getLastShow());
        strip.show();
    }
}

void SphereController::initLedSettings()
{

    bool fsinit = false;
    DEBUGFS_PRINTLN(F("Mounting FS"));
    fsinit = LittleFS.begin(true);
    if (!fsinit)
    {
        DEBUGFS_PRINTLN(F("FS failed!"));
        return;
    }
    updateFSInfo();

    DEBUG_PRINTLN(F("Reading config"));
    deserializeConfigFromFS();
}

void SphereController::beginStrip()
{
    // Initialize NeoPixel Strip and button
    strip.finalizeInit(); // busses created during deserializeConfig()
    strip.makeAutoSegments();
    strip.setBrightness(0);

    if (true)
    {
        if (briS > 0)
            bri = briS;
        else if (bri == 0)
            bri = 128;
    }
}

void SphereController::initBusses()
{
    DEBUG_PRINTLN(F("Re-init busses."));
    bool aligned = strip.checkSegmentAlignment(); // see if old segments match old bus(ses)
    busses.removeAll();
    uint32_t mem = 0, globalBufMem = 0;
    uint16_t maxlen = 0;
    for (uint8_t i = 0; i < MAX_PINS; i++)
    {
        if (busConfigs[i] == nullptr)
            break;
        mem += MyNamespace::BusManager::memUsage(*busConfigs[i]);
        if (useGlobalLedBuffer && busConfigs[i]->start + busConfigs[i]->count > maxlen)
        {
            maxlen = busConfigs[i]->start + busConfigs[i]->count;
            globalBufMem = maxlen * 4;
        }
        if (mem + globalBufMem <= MAX_LED_MEMORY)
        {
            busses.add(*busConfigs[i]);
        }
        delete busConfigs[i];
        busConfigs[i] = nullptr;
    }
    strip.finalizeInit(); // also loads default ledmap if present
    if (aligned)
        strip.makeAutoSegments();
    else
        strip.fixInvalidSegments();
}