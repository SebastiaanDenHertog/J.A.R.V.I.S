#include "model_runner.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/tools/logging.h"
#include <iostream>
#include <cstring>

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

bool ModelRunner::CreateInterpreter()
{
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model_, resolver);
    if (builder(&interpreter_) != kTfLiteOk)
    {
        std::cerr << "Failed to build interpreter." << std::endl;
        return false;
    }
    return true;
}

ModelRunner::ModelRunner(const std::string &model_path)
{
    model_ = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if (!model_)
    {
        std::cerr << "Failed to load model." << std::endl;
    }
    else if (!CreateInterpreter())
    {
        std::cerr << "Failed to create interpreter." << std::endl;
        model_.reset();
    }
    else if (interpreter_->AllocateTensors() != kTfLiteOk)
    {
        std::cerr << "Failed to allocate tensors!" << std::endl;
        model_.reset();
        interpreter_.reset();
    }
}

bool ModelRunner::IsLoaded() const
{
    return model_ && interpreter_;
}

float ModelRunner::RunModel(float input_data)
{
    if (!IsLoaded())
    {
        std::cerr << "Model is not loaded properly." << std::endl;
        return 0.0f;
    }

    std::memcpy(interpreter_->typed_input_tensor<float>(0), &input_data, sizeof(float));

    if (interpreter_->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke!" << std::endl;
        return 0.0f;
    }

    return *interpreter_->typed_output_tensor<float>(0);
}

std::vector<float> ModelRunner::RunModel(const std::vector<float> &input_data)
{
    if (!IsLoaded())
    {
        std::cerr << "Model is not loaded properly." << std::endl;
        return {};
    }

    float *input = interpreter_->typed_input_tensor<float>(0);
    std::memcpy(input, input_data.data(), input_data.size() * sizeof(float));

    if (interpreter_->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke!" << std::endl;
        return {};
    }

    const float *output = interpreter_->typed_output_tensor<float>(0);
    return std::vector<float>(output, output + interpreter_->tensor(interpreter_->outputs()[0])->bytes / sizeof(float));
}

void ModelRunner::modelsLogic(SoundData *soundData)
{

    // Load the TFLite models

    if (!model_)
    {
        std::cerr << "Failed to load model" << std::endl;
        return;
    }

    // Build the interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder(*model_, resolver)(&interpreter);
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

    // Preprocess audio data
    std::vector<float> processedAudio = PreprocessAudioData(soundData->data, soundData->length);

    // Copy preprocessed data to input tensor
    float *input = interpreter->typed_tensor<
        float>(interpreter->inputs()[0]);
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
}