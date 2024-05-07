#include "ReSpeaker.h"
#include "wifi_server.h"
#include "Pixel_Ring.h"
#include "model_runner.h"
#include <vector>
#include <iostream>

// A mock audio preprocessing function that converts audio data to float.
std::vector<float> preprocessAudioData(uint8_t *audioData, uint32_t dataLength)
{
    std::vector<float> floatAudioData;
    floatAudioData.reserve(dataLength);

    // Normalize audio data to the range [-1, 1] or [0, 1] depending on the requirement.
    for (uint32_t i = 0; i < dataLength; ++i)
    {
        float normalizedValue = static_cast<float>(audioData[i]) / 255.0f; // Example normalization
        floatAudioData.push_back(normalizedValue);
    }

    return floatAudioData;
}

// Run all models and logic in a function to avoid global variables.
void modelsLogic(uint8_t *audioData, uint32_t dataLength)
{
    // Load models.
    ModelRunner speechModel("models/whisper_english.tflite");
    ModelRunner nlpModel("models/nlp_model.tflite");
    ModelRunner llmModel("models/llm_model.tflite");

    // Preprocess audio data.
    std::vector<float> processedAudio = preprocessAudioData(audioData, dataLength);

    // Run models. Ensure the input type matches.
    float speechOutput = speechModel.RunModel(processedAudio[0]); // Adjust as per the model's input needs.
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
    PixelRing pixelring("/dev/spidev0.0", 12);
    try
    {
        const char *devicePath = "/dev/i2c-1";
        uint8_t deviceAddress = 0x40;
        uint8_t micCount = 4;

        pixelring.setBrightness(15);
        pixelring.startAnimation();

        ReSpeaker respeaker(devicePath, deviceAddress, micCount);
        wifiServer wifiserver(port, respeaker);

        wifiserver.run();
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
