#include "ModelRunner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/core/types.hpp>
#include <tensorflow/lite/kernels/register.h> // Include for BuiltinOpResolver

ModelRunner::ModelRunner(const std::unordered_map<std::string, std::string> &model_paths)
{
    for (const auto &model : model_paths)
    {
        if (!LoadModel(model.first, model.second))
        {
            std::cerr << "Failed to load model: " << model.first << " from path: " << model.second << std::endl;
        }
    }
}

bool ModelRunner::LoadModel(const std::string &model_name, const std::string &model_path)
{
    auto model = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if (!model)
    {
        std::cerr << "Failed to load model from path: " << model_path << std::endl;
        return false;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;

    if (builder(&interpreter) != kTfLiteOk)
    {
        std::cerr << "Failed to build interpreter for model: " << model_name << std::endl;
        return false;
    }

    if (interpreter->AllocateTensors() != kTfLiteOk)
    {
        std::cerr << "Failed to allocate tensors for model: " << model_name << std::endl;
        return false;
    }

    models_[model_name] = std::move(model);
    interpreters_[model_name] = std::move(interpreter);

    return true;
}

bool ModelRunner::IsLoaded(const std::string &model_name) const
{
    return models_.find(model_name) != models_.end();
}

bool ModelRunner::RunInference(const std::string &model_name, const std::string &input_text, std::vector<float> &result)
{
    auto interpreter_it = interpreters_.find(model_name);
    if (interpreter_it == interpreters_.end())
    {
        std::cerr << "Model is not loaded properly." << std::endl;
        return false;
    }

    auto &interpreter = interpreter_it->second;
    auto &tokenizer = tokenizers_[model_name];

    auto *input_tensor = interpreter->typed_tensor<int>(interpreter->inputs()[0]);
    auto tokenized_input = prepare_input(input_text, tokenizer, 20);

    if (tokenized_input.empty())
    {
        std::cerr << "Tokenized input is empty." << std::endl;
        return false;
    }

    std::copy(tokenized_input.begin(), tokenized_input.end(), input_tensor);

    if (interpreter->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to run the model." << std::endl;
        return false;
    }

    auto *output_tensor = interpreter->tensor(interpreter->outputs()[0]);
    if (output_tensor == nullptr)
    {
        std::cerr << "Output tensor is null." << std::endl;
        return false;
    }

    if (output_tensor->type != kTfLiteFloat32)
    {
        std::cerr << "Unsupported output tensor type." << std::endl;
        return false;
    }

    const float *output = interpreter->typed_output_tensor<float>(0);
    result.assign(output, output + output_tensor->dims->data[1]);

    std::cerr << "Model output values: ";
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cerr << result[i] << " ";
    }
    std::cerr << std::endl;

    return true;
}

std::string ModelRunner::predictTaskFromInput(const std::string &model_name, const std::string &input)
{
    std::vector<float> results;
    if (!RunInference(model_name, input, results) || results.empty())
    {
        return "Failed to run NLP model.";
    }

    auto max_iter = std::max_element(results.begin(), results.end());
    int predicted_class = std::distance(results.begin(), max_iter);
    float confidence = *max_iter;

    std::string task_description = "Predicted task: ";
    if (class_names_.count(model_name) > 0 && predicted_class < class_names_[model_name].size())
    {
        task_description += class_names_[model_name][predicted_class];
    }
    else
    {
        task_description += "Unknown";
    }

    std::ostringstream oss;
    oss << task_description << " with confidence " << confidence;
    return oss.str();
}

void ModelRunner::LoadClassNames(const std::unordered_map<std::string, std::string> &class_files)
{
    std::cerr << "Loading class names..." << std::endl;
    std::cerr << "Class files: " << class_files.size() << std::endl;
    for (const auto &pair : class_files)
    {
        cv::FileStorage fs(pair.second, cv::FileStorage::READ);
        if (!fs.isOpened())
        {
        std::cerr << "Failed to open " << pair.second << std::endl;
        return;
        }
        cv::FileNode n = fs[pair.first]; // Read string sequence - Get node
        if (n.type() != cv::FileNode::SEQ)
        {
        std::cerr << pair.first << " is not a sequence! FAIL" << std::endl;
        return ;
        }
        cv::FileNodeIterator it = n.begin(), it_end = n.end(); // Go through the node
        for (; it != it_end; ++it)
        std::cout << (std::string)*it << std::endl;
        }

    std::cerr << "Class names loaded successfully." << std::endl;
}

std::vector<int> ModelRunner::prepare_input(const std::string &input_text, const std::unordered_map<std::string, int> &tokenizer, int max_length)
{
    std::vector<int> tokenized_input(max_length, 0);
    std::istringstream iss(input_text);
    std::string token;
    int idx = 0;
    while (iss >> token && idx < max_length)
    {
        auto it = tokenizer.find(token);
        if (it != tokenizer.end())
        {
            tokenized_input[idx++] = it->second;
        }
    }
    return tokenized_input;
}
