#include "ModelRunner.h"
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/interpreter_builder.h>
#include <iostream>
#include <fstream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "Tokenizer.h"

// Utility function to prepare input
std::vector<int> prepare_input(const std::string &text, Tokenizer &tokenizer, size_t sequence_length)
{
    std::vector<int> sequence = tokenizer.texts_to_sequences(text);
    if (sequence.size() > sequence_length)
    {
        sequence.resize(sequence_length);
    }
    else
    {
        sequence.resize(sequence_length, 0); // Padding with 0s
    }
    return sequence;
}

ModelRunner::ModelRunner(const std::string &model_path)
{
    std::cerr << "Loading model from: " << model_path << std::endl;
    model_ = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if (!model_)
    {
        std::cerr << "Failed to load model: " << model_path << std::endl;
        return;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model_, resolver);

    if (builder(&interpreter_) != kTfLiteOk)
    {
        std::cerr << "Failed to build interpreter." << std::endl;
        return;
    }

    if (interpreter_->AllocateTensors() != kTfLiteOk)
    {
        std::cerr << "Failed to allocate tensors." << std::endl;
        return;
    }

    // Print input tensor details
    const TfLiteTensor *input_tensor = interpreter_->input_tensor(0);
    std::cerr << "Input Tensor Details:" << std::endl;
    std::cerr << "  Type: " << input_tensor->type << std::endl;
    std::cerr << "  Dimensions: " << input_tensor->dims->size << std::endl;
    for (int i = 0; i < input_tensor->dims->size; ++i)
    {
        std::cerr << "    Dim " << i << ": " << input_tensor->dims->data[i] << std::endl;
    }

    // Print output tensor details
    const TfLiteTensor *output_tensor = interpreter_->output_tensor(0);
    std::cerr << "Output Tensor Details:" << std::endl;
    std::cerr << "  Type: " << output_tensor->type << std::endl;
    std::cerr << "  Dimensions: " << output_tensor->dims->size << std::endl;
    for (int i = 0; i < output_tensor->dims->size; ++i)
    {
        std::cerr << "    Dim " << i << ": " << output_tensor->dims->data[i] << std::endl;
    }

    std::cerr << "Model loaded successfully." << std::endl;
}

bool ModelRunner::IsLoaded() const
{
    return model_ != nullptr && interpreter_ != nullptr;
}

void ModelRunner::LoadClassNames(const std::string &file_path)
{
    std::cerr << "Loading class names from: " << file_path << std::endl;

    cv::FileStorage fs(file_path, cv::FileStorage::READ);
    if (!fs.isOpened())
    {
        std::cerr << "Failed to open class names file: " << file_path << std::endl;
        return;
    }

    cv::FileNode class_names_node = fs["class_names"];
    if (class_names_node.type() != cv::FileNode::SEQ)
    {
        std::cerr << "Class names file is not in expected format." << std::endl;
        return;
    }

    class_names_.clear();
    for (cv::FileNodeIterator it = class_names_node.begin(); it != class_names_node.end(); ++it)
    {
        std::string class_name;
        *it >> class_name;
        class_names_.push_back(class_name);
    }

    fs.release();

    std::cerr << "Class names loaded successfully." << std::endl;

    // Print the loaded class names for verification
    std::cerr << "Class names: ";
    for (const auto &name : class_names_)
    {
        std::cerr << name << " ";
    }
    std::cerr << std::endl;
}

bool ModelRunner::RunInference(const std::string &input_text, std::vector<float> &result)
{
    if (!IsLoaded())
    {
        std::cerr << "Model is not loaded properly." << std::endl;
        return false;
    }

    // Initialize the tokenizer and fit on some sample texts
    Tokenizer tokenizer;
    std::vector<std::string> sample_texts = {"can you turn on the lights", "please turn off the lights"};
    tokenizer.fit_on_texts(sample_texts);

    // Prepare the input
    std::vector<int> input_data = prepare_input(input_text, tokenizer, 20); // Assuming max length is 20

    // Debug: Print tokenized input
    std::cerr << "Tokenized input: ";
    for (const auto &val : input_data)
    {
        std::cerr << val << " ";
    }
    std::cerr << std::endl;

    // Ensure the input tensor is of the correct size
    interpreter_->ResizeInputTensor(interpreter_->inputs()[0], {1, static_cast<int>(input_data.size())});
    if (interpreter_->AllocateTensors() != kTfLiteOk)
    {
        std::cerr << "Failed to allocate tensors." << std::endl;
        return false;
    }

    // Copy input data to the input tensor
    int *input_tensor_data = interpreter_->typed_input_tensor<int>(0);
    std::memcpy(input_tensor_data, input_data.data(), input_data.size() * sizeof(int));

    // Debug: Print input tensor data
    std::cerr << "Input tensor values: ";
    for (const auto &val : input_data)
    {
        std::cerr << val << " ";
    }
    std::cerr << std::endl;

    // Invoke the model
    if (interpreter_->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke model." << std::endl;
        return false;
    }

    // Get output tensor data
    const TfLiteTensor *output_tensor = interpreter_->tensor(interpreter_->outputs()[0]);
    if (!output_tensor)
    {
        std::cerr << "Output tensor is null." << std::endl;
        return false;
    }

    // Print output tensor details
    std::cerr << "Output Tensor Details:" << std::endl;
    std::cerr << "  Type: " << output_tensor->type << std::endl;
    std::cerr << "  Dimensions: " << output_tensor->dims->size << std::endl;
    for (int i = 0; i < output_tensor->dims->size; ++i)
    {
        std::cerr << "    Dim " << i << ": " << output_tensor->dims->data[i] << std::endl;
    }

    // Extract output data based on the type
    switch (output_tensor->type)
    {
    case kTfLiteFloat32:
    {
        int output_size = output_tensor->dims->data[output_tensor->dims->size - 1];
        result.resize(output_size);
        const float *output_data = interpreter_->typed_output_tensor<float>(0);
        std::copy(output_data, output_data + output_size, result.begin());
        break;
    }
    default:
        std::cerr << "Unsupported output tensor type." << std::endl;
        return false;
    }

    // Debug: Print raw output values
    std::cerr << "Model raw output values: ";
    for (const auto &val : result)
    {
        std::cerr << val << " ";
    }
    std::cerr << std::endl;

    return true;
}

Task ModelRunner::predictTaskFromInput(const std::string &input)
{
    std::vector<float> nlp_result;
    if (!RunInference(input, nlp_result) || nlp_result.empty())
    {
        std::cerr << "Failed to run NLP model." << std::endl;
        return Task("Failed to run NLP model.", 0, Task::ERROR);
    }

    // Find the class with the highest probability
    auto max_it = std::max_element(nlp_result.begin(), nlp_result.end());
    int predicted_class = std::distance(nlp_result.begin(), max_it);
    float confidence = *max_it;

    std::string task_description = "Predicted task for class " + std::to_string(predicted_class) + " with confidence " + std::to_string(confidence);

    if (!class_names_.empty())
    {
        task_description = "Predicted task: " + class_names_[predicted_class] + " with confidence " + std::to_string(confidence);
    }

    // Debug: Print predicted task
    std::cerr << "Predicted task: " << task_description << std::endl;

    // Create a Task object based on the predicted class
    Task predicted_task(task_description, predicted_class, Task::GENERAL);

    return predicted_task;
}
