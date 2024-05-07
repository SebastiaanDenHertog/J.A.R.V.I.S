#include "ReSpeaker.h"
#include "wifi_server.h"
#include "Pixel_Ring.h"
#include "model_runner.h"

// run all model and logic in a funtion to avoid global variables
// model: nlp, llm,
void modelsLogic(uint8_t *audioData, uint32_t dataLength)
{
    // load models
    ModelRunner speechModel("models/whisper_english.tflite");
    ModelRunner nlpModel("models/nlp_model.tflite");
    ModelRunner llmModel("models/llm_model.tflite");

    // run models
    float speechOutput = speechModel.RunModel(audioData);
    float nlpOutput = nlpModel.RunModel(speechOutput);
    std::cout << "NLP output: " << nlpOutput << std::endl;
    float llmOutput = llmModel.RunModel(0.5f);

    // print output
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
