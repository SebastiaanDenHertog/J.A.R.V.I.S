#include "ReSpeaker.h"
#include "wifi_server.h"
#include "Pixel_Ring.h"
#include "model_runner.h"
#include "DeviceScanner.h"
#include <vector>
#include <iostream>
#include <thread>

// A mock audio preprocessing function that converts audio data to float.
float preprocessAudioData(uint8_t *audioData, uint32_t dataLength)
{
    float normalizedValue = static_cast<float>(*audioData) / 255.0f; // Example normalization
    return normalizedValue;
}

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

    // Run models. Ensure the input type matches.
    float dnnOutput = dnnModel.RunModel(processedAudio);
    std::cout << "dnn output: " << dnnOutput << std::endl;

    float speechOutput = speechModel.RunModel(processedAudio);
    std::cout << "Speech output: " << speechOutput << std::endl;
    float nlpOutput = nlpModel.RunModel(speechOutput);

    std::cout << "NLP output: " << nlpOutput << std::endl;
    float llmOutput = llmModel.RunModel(0.5f);

    // Print output.
    std::cout << "NLP output: " << nlpOutput << std::endl;
    std::cout << "LLM output: " << llmOutput << std::endl;
}

int main(int argc, char *argv[])
{
    int port = 8080;
    const char *devicePath = "/dev/i2c-1";
    uint8_t deviceAddress = 0x3b;
    uint8_t micCount = 4;
    uint8_t ledCound = 12;
    PixelRing pixelring(devicePath, deviceAddress, ledCound);
    ReSpeaker respeaker(devicePath, deviceAddress, micCount);
    wifiServer wifiserver(port, respeaker);
    try
    {
        pixelring.setBrightness(15);
        pixelring.startAnimation();
        std::thread t1(&wifiServer::run, &wifiserver); // Fix: Use function pointer and pass object as argument
        t1.detach();
        uint32_t dataLength;
        uint8_t *audioData = respeaker.startCaptureAndGetAudioData(dataLength);
        if (audioData != nullptr)
        {
            modelsLogic(audioData, dataLength);
            delete[] audioData;
        }
        respeaker.stopCapture();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    pixelring.stopAnimation();
    return 0;
}
