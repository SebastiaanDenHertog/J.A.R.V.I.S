#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/tools/logging.h"

#include <iostream>
#include <memory>

// Helper function to create an interpreter
std::unique_ptr<tflite::Interpreter> CreateInterpreter(const tflite::Model &model)
{
    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder builder(model, resolver);
    if (builder(&interpreter) != kTfLiteOk)
    {
        std::cerr << "Failed to build interpreter." << std::endl;
        return nullptr;
    }
    return interpreter;
}

// Main function
int main()
{
    // Load the model
    const char *model_path = "path_to_your_model.tflite";
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(model_path);
    if (!model)
    {
        std::cerr << "Failed to load model." << std::endl;
        return 1;
    }

    // Create an interpreter
    std::unique_ptr<tflite::Interpreter> interpreter = CreateInterpreter(*model);
    if (!interpreter)
    {
        std::cerr << "Failed to create interpreter." << std::endl;
        return 1;
    }

    // Allocate tensors
    if (interpreter->AllocateTensors() != kTfLiteOk)
    {
        std::cerr << "Failed to allocate tensors!" << std::endl;
        return 1;
    }

    // Assuming input and output tensors are accessible and properly configured
    // Example: Set input data
    // This highly depends on your model's input. Here it's assumed to be a single float.
    float input_data = 0.0f; // Replace with your input
    std::memcpy(interpreter->typed_input_tensor<float>(0), &input_data, sizeof(float));

    // Run inference
    if (interpreter->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke!" << std::endl;
        return 1;
    }

    // Get output data
    // This also depends on your model. Assuming it's a single float output.
    float output_data = *interpreter->typed_output_tensor<float>(0);
    std::cout << "Model output: " << output_data << std::endl;

    return 0;
}
