#include <iostream>
#include <thread>
#include "BluetoothComm.h"
#include "Wifi.h"
#include "Pixel_Ring.h"
#include "ReSpeaker.h"
#include "model_runner.h"
#include "authorization_api.h"
#include "web_service.h"
#include <boost/make_shared.hpp>

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

int port = 8080;
const char *devicePath = "/dev/i2c-1";
uint8_t deviceAddress = 0x3b;
uint8_t micCount = 4;
uint8_t ledCount = 12;

float preprocessAudioData(uint8_t *audioData, uint32_t dataLength)
{
    float normalizedValue = static_cast<float>(*audioData) / 255.0f;
    return normalizedValue;
}

std::vector<float> preprocessAudioToVector(uint8_t *audioData, uint32_t dataLength)
{
    return std::vector<float>(dataLength, 0.5f);
}

void modelsLogic(uint8_t *audioData, uint32_t dataLength)
{
    DEBUG_PRINT("Starting modelsLogic function.");

    ModelRunner dnnModel("models/dnn_model.tflite");
    ModelRunner speechModel("models/whisper_english.tflite");
    ModelRunner nlpModel("models/nlp_model.tflite");
    ModelRunner llmModel("models/llm_model.tflite");

    DEBUG_PRINT("Models loaded.");

    float processedAudio = preprocessAudioData(audioData, dataLength);
    std::vector<float> processedAudioVector = preprocessAudioToVector(audioData, dataLength);

    DEBUG_PRINT("Audio data preprocessed.");

    float dnnOutput = dnnModel.RunModel(processedAudio);
    DEBUG_PRINT("dnn output: " << dnnOutput);

    std::vector<float> speechOutput = speechModel.RunModel(processedAudioVector);
    DEBUG_PRINT("Speech output: ");
    for (const auto &val : speechOutput)
    {
        DEBUG_PRINT(val << " ");
    }

    std::vector<float> nlpOutput = nlpModel.RunModel(speechOutput);
    DEBUG_PRINT("NLP output: ");
    for (const auto &val : nlpOutput)
    {
        DEBUG_PRINT(val << " ");
    }

    std::vector<float> llmOutput = llmModel.RunModel({0.5f});
    DEBUG_PRINT("LLM output: ");
    for (const auto &val : llmOutput)
    {
        DEBUG_PRINT(val << " ");
    }

    DEBUG_PRINT("Completed modelsLogic function.");
}

bool checkBluetoothAvailability();

int main(int argc, char *argv[])
{
#ifdef DEBUG_MODE
    std::cout << "Debug mode is ON" << std::endl;
#endif

    // Check command line arguments.
    if (std::getenv("PORT") == NULL || std::getenv("THREADS") == NULL)
    {
        std::cerr << "Usage: web_service\n"
                  << "Please define the PORT and THREADS environment variables\n";
        return EXIT_FAILURE;
    }

    unsigned short web_server_port = static_cast<unsigned short>(std::atoi(std::getenv("PORT")));
    int threads = std::max<int>(1, std::atoi(std::getenv("THREADS")));

    std::unordered_set<std::string> allowed_keys;
    allowed_keys.insert("SampleKey");
    boost::shared_ptr<authorization_api> auth = boost::make_shared<authorization_api>(allowed_keys);
    boost::shared_ptr<web_service_context> ctx = boost::make_shared<web_service_context>(threads, auth);

    std::make_shared<web_service>(
        ctx,
        "0.0.0.0",
        web_server_port,
        "web_service")
        ->run();

    if (!checkBluetoothAvailability())
    {
        std::cerr << "Bluetooth is not available on this device." << std::endl;
        return -1;
    }
    DEBUG_PRINT("Bluetooth is available.");

    BluetoothComm btComm;
    if (!btComm.initialize())
    {
        std::cerr << "Failed to initialize Bluetooth communication." << std::endl;
        return -1;
    }
    DEBUG_PRINT("Bluetooth communication initialized.");

    std::thread bluetoothThread(&BluetoothComm::handleIncomingConnectionsThread, &btComm);
    DEBUG_PRINT("Bluetooth thread started.");

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

    PixelRing pixelring(devicePath, deviceAddress, ledCount);
    ReSpeaker respeaker(devicePath, deviceAddress, micCount);
    respeaker.initBoard();
    DEBUG_PRINT("ReSpeaker initialized.");

    modelsLogic(respeaker.startCaptureAndGetAudioData(), 1024);
    DEBUG_PRINT("modelsLogic executed.");

    pixelring.setBrightness(15);
    pixelring.startAnimation();
    DEBUG_PRINT("PixelRing animation started.");

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ctx]
            {
                ctx->get_ioc()->run();
            });
    ctx->get_ioc()->run();

    while (true)
    {
    }

    bluetoothThread.join();
    btComm.terminate();
    DEBUG_PRINT("Bluetooth thread joined and communication terminated.");

    std::cout << "Application finished." << std::endl;
    return 0;
}
