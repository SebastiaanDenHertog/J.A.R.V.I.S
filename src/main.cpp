#include <iostream>
#include <thread>
#include "BluetoothComm.h"
#include "Wifi.h"

#if defined(SERVER_BUILD)
#include "model_runner.h"
#elif defined(CLIENT_BUILD)
#include "Pixel_Ring.h"
#include "ReSpeaker.h"
#include "ClientSpecificHeader.h" // Placeholder for actual client-specific headers
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

#if defined(SERVER_BUILD)
// Run all models and logic in a function to avoid global variables.
void modelsLogic(uint8_t *audioData, uint32_t dataLength)
{
    // Load models.
    ModelRunner dnnModel("models/dnn_model.tflite");
    ModelRunner speechModel("models/whisper_english.tflite");
    ModelRunner nlpModel("models/nlp_model.tflite");
    ModelRunner llmModel("models/llm_model.tflite");

    // Preprocess audio data.
    float processedAudio = preprocessAudioData(audioData, dataLength);
    std::vector<float> processedAudioVector = preprocessAudioToVector(audioData, dataLength);

    // Run models. Ensure the input type matches.
    float dnnOutput = dnnModel.RunModel(processedAudio);
    std::cout << "dnn output: " << dnnOutput << std::endl;

    std::vector<float> speechOutput = speechModel.RunModel(processedAudioVector);
    std::cout << "Speech output: ";
    for (const auto& val : speechOutput) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::vector<float> nlpOutput = nlpModel.RunModel(speechOutput);
    std::cout << "NLP output: ";
    for (const auto& val : nlpOutput) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::vector<float> llmOutput = llmModel.RunModel({0.5f}); // Placeholder input
    std::cout << "LLM output: ";
    for (const auto& val : llmOutput) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
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

void runCommonFunctionality()
{
    // Check if the device has Bluetooth capabilities
    if (!checkBluetoothAvailability())
    {
        std::cerr << "Bluetooth is not available on this device." << std::endl;
        return;
    }

    BluetoothComm btComm;

    // Initialize Bluetooth communication
    if (!btComm.initialize())
    {
        std::cerr << "Failed to initialize Bluetooth communication." << std::endl;
        return;
    }

    std::thread createConnectionsThread(&BluetoothComm::createConnectionsThread, &btComm);
    std::thread handleIncomingConnectionsThread(&BluetoothComm::handleIncomingConnectionsThread, &btComm);

    createConnectionsThread.join();
    handleIncomingConnectionsThread.join();

    btComm.terminate();
}

int main(int argc, char *argv[])
{
    int port = 8080;
    const char *devicePath = "/dev/i2c-1";
    uint8_t deviceAddress = 0x3b;
    uint8_t micCount = 4;
    uint8_t ledCount = 12;

#if defined(SERVER_BUILD)
    PixelRing pixelring(devicePath, deviceAddress, ledCount);
    ReSpeaker respeaker(devicePath, deviceAddress, micCount);
    ReSpeaker.initBoard();
    modelsLogic(ReSpeaker.startCaptureAndGetAudioData(), 1024);
    wifiServer wifiserver(port);
    try
    {
        pixelring.setBrightness(15);
        pixelring.startAnimation();
        std::thread t1(&wifiServer::run, &wifiserver); // Fix: Use function pointer and pass object as argument
        t1.detach();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    pixelring.stopAnimation();
#elif defined(CLIENT_BUILD)
    wificlient wificlient(port);
    std::cout << "Client build: Initializing client components..." << std::endl;
#endif
    runCommonFunctionality();

    std::cout << "Application finished." << std::endl;
    return 0;
}
