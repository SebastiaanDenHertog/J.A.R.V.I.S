#include <iostream>
#include <thread>
#include "BluetoothComm.h"
#include "Wifi.h"

#if defined(SERVER_BUILD) || defined(FULL_BUILD)
#include "model_runner.h"
#endif

#if defined(CLIENT_BUILD) || defined(FULL_BUILD)
#include "Pixel_Ring.h"
#include "ReSpeaker.h"
#include "ClientSpecificHeader.h" // Placeholder for actual client-specific headers
#endif

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

// A mock audio preprocessing function that converts audio data to float.
float preprocessAudioData(uint8_t *audioData, uint32_t dataLength)
{
    float normalizedValue = static_cast<float>(*audioData) / 255.0f; // Example normalization
    return normalizedValue;
}

std::vector<float> preprocessAudioToVector(uint8_t* audioData, uint32_t dataLength)
{
    // Implement actual preprocessing logic here
    return std::vector<float>(dataLength, 0.5f); // Placeholder
}

#if defined(SERVER_BUILD) || defined(FULL_BUILD)
// Run all models and logic in a function to avoid global variables.
void modelsLogic(uint8_t *audioData, uint32_t dataLength)
{
    DEBUG_PRINT("Starting modelsLogic function.");

    // Load models.
    ModelRunner dnnModel("models/dnn_model.tflite");
    ModelRunner speechModel("models/whisper_english.tflite");
    ModelRunner nlpModel("models/nlp_model.tflite");
    ModelRunner llmModel("models/llm_model.tflite");

    DEBUG_PRINT("Models loaded.");

    // Preprocess audio data.
    float processedAudio = preprocessAudioData(audioData, dataLength);
    std::vector<float> processedAudioVector = preprocessAudioToVector(audioData, dataLength);

    DEBUG_PRINT("Audio data preprocessed.");

    // Run models. Ensure the input type matches.
    float dnnOutput = dnnModel.RunModel(processedAudio);
    DEBUG_PRINT("dnn output: " << dnnOutput);

    std::vector<float> speechOutput = speechModel.RunModel(processedAudioVector);
    DEBUG_PRINT("Speech output: ");
    for (const auto& val : speechOutput) {
        DEBUG_PRINT(val << " ");
    }

    std::vector<float> nlpOutput = nlpModel.RunModel(speechOutput);
    DEBUG_PRINT("NLP output: ");
    for (const auto& val : nlpOutput) {
        DEBUG_PRINT(val << " ");
    }

    std::vector<float> llmOutput = llmModel.RunModel({0.5f}); // Placeholder input
    DEBUG_PRINT("LLM output: ");
    for (const auto& val : llmOutput) {
        DEBUG_PRINT(val << " ");
    }

    DEBUG_PRINT("Completed modelsLogic function.");
}
#endif

bool checkBluetoothAvailability()
{
    int dev_id = hci_get_route(nullptr);
    int sock = hci_open_dev(dev_id);
    if (dev_id < 0 || sock < 0)
    {
        return false;
    }
    close(sock);
    return true;
}

int main(int argc, char *argv[])
{
#ifdef DEBUG_MODE
    std::cout << "Debug mode is ON" << std::endl;
#endif

    int port = 8080;
    const char *devicePath = "/dev/i2c-1";
    uint8_t deviceAddress = 0x3b;
    uint8_t micCount = 4;
    uint8_t ledCount = 12;

    // Check if the device has Bluetooth capabilities
    if (!checkBluetoothAvailability())
    {
        std::cerr << "Bluetooth is not available on this device." << std::endl;
        return -1;
    }
    DEBUG_PRINT("Bluetooth is available.");

    BluetoothComm btComm;

    // Initialize Bluetooth communication
    if (!btComm.initialize())
    {
        std::cerr << "Failed to initialize Bluetooth communication." << std::endl;
        return -1;
    }
    DEBUG_PRINT("Bluetooth communication initialized.");

    // Start Bluetooth thread
    std::thread bluetoothThread(&BluetoothComm::handleIncomingConnectionsThread, &btComm);
    DEBUG_PRINT("Bluetooth thread started.");

#if defined(SERVER_BUILD) || defined(FULL_BUILD)
    wifiServer wifiserver(port);
    try
    {
        DEBUG_PRINT("Starting wifiServer.");
        wifiserver.run();
        DEBUG_PRINT("wifiServer running.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
#endif

#if defined(CLIENT_BUILD) || defined(FULL_BUILD)
    PixelRing pixelring(devicePath, deviceAddress, ledCount);
    ReSpeaker respeaker(devicePath, deviceAddress, micCount);
    respeaker.initBoard();
    DEBUG_PRINT("ReSpeaker initialized.");
    
    modelsLogic(respeaker.startCaptureAndGetAudioData(), 1024);
    DEBUG_PRINT("modelsLogic executed.");

    pixelring.setBrightness(15);
    pixelring.startAnimation();
    DEBUG_PRINT("PixelRing animation started.");

    // Keep the client task running
    while (true)
    {
        // Add client-specific periodic tasks here
    }
#endif

    // Main loop or additional functionality
    while (true)
    {
        // Add any periodic checks or functionality here
    }

    bluetoothThread.join();
    btComm.terminate();
    DEBUG_PRINT("Bluetooth thread joined and communication terminated.");

    std::cout << "Application finished." << std::endl;
    return 0;
}
