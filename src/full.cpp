#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <unordered_set>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/optional_debug_tools.h>
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
bool setbluetooth = false;

bool checkBluetoothAvailability()
{
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0)
    {
        return false;
    }

    int sock = hci_open_dev(dev_id);
    if (sock < 0)
    {
        return false;
    }

    hci_close_dev(sock);
    return true;
}

std::vector<float> PreprocessAudioData(uint8_t *audioData, uint32_t dataLength)
{
    std::vector<float> processedData;
    // Example: Convert uint8_t data to float and normalize (assuming 8-bit PCM)
    for (uint32_t i = 0; i < dataLength; ++i)
    {
        processedData.push_back(static_cast<float>(audioData[i]) / 255.0f);
    }
    // Further processing like resampling, MFCC extraction can be added here
    return processedData;
}

std::string PostprocessOutput(const std::vector<float> &outputData)
{
    // Example: Convert the output data to a string
    // This will depend on your specific model's output format
    std::string result;
    // Assuming outputData contains character indices or probabilities
    for (float val : outputData)
    {
        char c = static_cast<char>(val); // Simplified for example
        result += c;
    }
    return result;
}

void modelsLogic(uint8_t *audioData, uint32_t dataLength)
{
    DEBUG_PRINT("Starting modelsLogic function.");

    // Load the TFLite models
    auto model = tflite::FlatBufferModel::BuildFromFile("models/whisper_english.tflite");
    if (!model)
    {
        std::cerr << "Failed to load model" << std::endl;
        return;
    }

    // Build the interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder(*model, resolver)(&interpreter);
    if (!interpreter)
    {
        std::cerr << "Failed to create interpreter" << std::endl;
        return;
    }

    // Allocate tensor buffers
    if (interpreter->AllocateTensors() != kTfLiteOk)
    {
        std::cerr << "Failed to allocate tensors" << std::endl;
        return;
    }

    DEBUG_PRINT("Models loaded.");

    // Preprocess audio data
    std::vector<float> processedAudio = PreprocessAudioData(audioData, dataLength);
    DEBUG_PRINT("Audio data preprocessed.");

    // Copy preprocessed data to input tensor
    float *input = interpreter->typed_tensor<float>(interpreter->inputs()[0]);
    std::copy(processedAudio.begin(), processedAudio.end(), input);

    // Run inference
    if (interpreter->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke TFLite interpreter" << std::endl;
        return;
    }

    // Process output
    auto output = interpreter->typed_output_tensor<float>(0);
    std::vector<float> outputData(output, output + interpreter->tensor(interpreter->outputs()[0])->bytes / sizeof(float));
    std::string result = PostprocessOutput(outputData);

    // Output result
    DEBUG_PRINT("Recognized text: " << result);

    DEBUG_PRINT("Completed modelsLogic function.");
}

int main(int argc, char *argv[])
{
#ifdef DEBUG_MODE
    std::cout << "Debug mode is ON" << std::endl;
#endif

    unsigned short web_server_port = 8081;
    int threads = 10;

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
    }
    else
    {
        DEBUG_PRINT("Bluetooth is available.");
        setbluetooth = true;
    }

 std::unique_ptr<BluetoothComm> bluetoothComm;
    std::thread bluetoothThread;

    if (!setbluetooth)
    {

    BluetoothComm btComm;
    if (!btComm.initialize())
    {
        std::cerr << "Failed to initialize Bluetooth communication." << std::endl;
        return -1;
    }
    DEBUG_PRINT("Bluetooth communication initialized.");

    std::thread bluetoothThread(&BluetoothComm::handleIncomingConnectionsThread, &btComm);
    DEBUG_PRINT("Bluetooth thread started.");
    }

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
    uint32_t bufferSize = 1024;
    uint8_t* rawAudioData = respeaker.startCaptureAndGetAudioData(bufferSize);

    std::vector<int16_t> audioData(rawAudioData, rawAudioData + bufferSize);

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

    if (bluetoothComm)
    {
        bluetoothThread.join();
        bluetoothComm->terminate();
        DEBUG_PRINT("Bluetooth thread joined and communication terminated.");
    }

    std::cout << "Application finished." << std::endl;
    return 0;
}
