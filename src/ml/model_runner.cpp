#include "model_runner.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/tools/logging.h"
#include <iostream>
#include <cstring>

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
