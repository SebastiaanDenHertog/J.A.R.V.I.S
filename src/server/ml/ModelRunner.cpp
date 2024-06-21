#include "ModelRunner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <algorithm>
#include <nlohmann/json.hpp>

ModelRunner::ModelRunner(const std::unordered_map<std::string, std::string> &model_paths)
{
    for (const auto &pair : model_paths)
    {
        const std::string &model_name = pair.first;
        const std::string &model_path = pair.second;

        models_[model_name] = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
        if (!models_[model_name])
        {
            throw std::runtime_error("Failed to load model: " + model_path);
        }

        tflite::ops::builtin::BuiltinOpResolver resolver;
        tflite::InterpreterBuilder(*models_[model_name], resolver)(&interpreters_[model_name]);
        if (!interpreters_[model_name])
        {
            throw std::runtime_error("Failed to build interpreter for model: " + model_name);
        }

        if (interpreters_[model_name]->AllocateTensors() != kTfLiteOk)
        {
            throw std::runtime_error("Failed to allocate tensors for model: " + model_name);
        }
    }
}

bool ModelRunner::IsLoaded(const std::string &model_name)
{
    return models_.find(model_name) != models_.end();
}

void ModelRunner::LoadTokenizer(const std::string &tokenizer_json_path)
{
    std::ifstream tokenizer_file(tokenizer_json_path);
    if (!tokenizer_file.is_open())
    {
        throw std::runtime_error("Failed to open tokenizer JSON file: " + tokenizer_json_path);
    }

    nlohmann::json tokenizer_json;
    tokenizer_file >> tokenizer_json;

    auto index_word = tokenizer_json["index_word"];
    for (auto it = index_word.begin(); it != index_word.end(); ++it)
    {
        int index = std::stoi(it.key());
        std::string word = it.value();
        tokenizer_index_word_[index] = word;
    }

    auto word_index = tokenizer_json["word_index"];
    for (auto it = word_index.begin(); it != word_index.end(); ++it)
    {
        std::string word = it.key();
        int index = it.value();
        tokenizer_word_index_[word] = index;
    }

    std::cout << "Loaded tokenizer with " << tokenizer_index_word_.size() << " words." << std::endl;
}

void ModelRunner::LoadLabels(const std::string &ner_labels_path, const std::string &command_type_labels_path)
{
    // Load NER labels
    std::ifstream ner_file(ner_labels_path);
    if (!ner_file.is_open())
    {
        throw std::runtime_error("Failed to open NER labels file: " + ner_labels_path);
    }

    nlohmann::json ner_json;
    ner_file >> ner_json;
    for (auto &[key, value] : ner_json.items())
    {
        int id = std::stoi(key);
        std::string label = value;
        ner_labels_[id] = label;
    }
    ner_file.close();
    std::cout << "Loaded NER labels: " << ner_labels_.size() << std::endl;

    // Load Command Type labels
    std::ifstream command_file(command_type_labels_path);
    if (!command_file.is_open())
    {
        throw std::runtime_error("Failed to open command type labels file: " + command_type_labels_path);
    }

    nlohmann::json command_json;
    command_file >> command_json;
    for (auto &[key, value] : command_json.items())
    {
        int id = std::stoi(key);
        std::string label = value;
        command_type_labels_[id] = label;
    }
    command_file.close();
    std::cout << "Loaded Command Type labels: " << command_type_labels_.size() << std::endl;
}

bool ModelRunner::RunInference(const std::string &model_name, const std::string &input_text, std::vector<float> &ner_result, std::vector<float> &command_type_result)
{
    if (!IsLoaded(model_name))
    {
        throw std::runtime_error("Model not loaded: " + model_name);
    }

    // Tokenize the input text
    std::vector<int> tokenized_input = TokenizeInput(input_text);

    // Get the interpreter
    auto &interpreter = interpreters_[model_name];

    // Get input tensor and copy the tokenized input to the input tensor
    TfLiteTensor *input_tensor = interpreter->tensor(interpreter->inputs()[0]);
    if (input_tensor == nullptr)
    {
        throw std::runtime_error("Failed to get input tensor");
    }

    std::memcpy(input_tensor->data.raw, tokenized_input.data(), tokenized_input.size() * sizeof(int));

    // Run the interpreter
    if (interpreter->Invoke() != kTfLiteOk)
    {
        throw std::runtime_error("Failed to invoke TFLite interpreter");
    }

    // Get NER output tensor and read the results
    TfLiteTensor *ner_output_tensor = interpreter->tensor(interpreter->outputs()[0]);
    ner_result.resize(ner_output_tensor->bytes / sizeof(float));
    std::memcpy(ner_result.data(), ner_output_tensor->data.raw, ner_output_tensor->bytes);

    // Get Command Type output tensor and read the results
    TfLiteTensor *command_type_output_tensor = interpreter->tensor(interpreter->outputs()[1]);
    command_type_result.resize(command_type_output_tensor->bytes / sizeof(float));
    std::memcpy(command_type_result.data(), command_type_output_tensor->data.raw, command_type_output_tensor->bytes);

    // Print the shape and data type of the output tensors for debugging
    std::cout << "NER Output Tensor: ";
    std::cout << "Shape: [";
    for (int i = 0; i < ner_output_tensor->dims->size; ++i)
    {
        std::cout << ner_output_tensor->dims->data[i];
        if (i < ner_output_tensor->dims->size - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "], Type: " << ner_output_tensor->type << std::endl;

    std::cout << "Command Type Output Tensor: ";
    std::cout << "Shape: [";
    for (int i = 0; i < command_type_output_tensor->dims->size; ++i)
    {
        std::cout << command_type_output_tensor->dims->data[i];
        if (i < command_type_output_tensor->dims->size - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "], Type: " << command_type_output_tensor->type << std::endl;

    return true;
}

std::vector<int> ModelRunner::TokenizeInput(const std::string &input_text)
{
    std::vector<int> tokenized_input(max_length_, 0); // Initialize with zeros
    std::istringstream iss(input_text);
    std::string word;
    int index = 0;

    while (iss >> word && index < max_length_)
    {
        if (tokenizer_.find(word) != tokenizer_.end())
        {
            tokenized_input[index++] = tokenizer_[word];
        }
        else
        {
            tokenized_input[index++] = tokenizer_["<UNK>"];
        }
    }

    return tokenized_input;
}

std::pair<std::string, std::string> ModelRunner::predictTaskFromInput(const std::string &model_name, const std::string &input)
{
    std::vector<float> ner_results;
    std::vector<float> command_type_results;

    RunInference(model_name, input, ner_results, command_type_results);

    // Debug output
    std::cout << "Raw NER results: ";
    for (const auto &res : ner_results)
    {
        std::cout << res << " ";
    }
    std::cout << std::endl;

    std::cout << "Raw Command Type results: ";
    for (const auto &res : command_type_results)
    {
        std::cout << res << " ";
    }
    std::cout << std::endl;

    // Find the index of the maximum value in the command type results
    int predicted_class_index = std::distance(command_type_results.begin(), std::max_element(command_type_results.begin(), command_type_results.end()));
    std::string task_description = "Unknown";

    if (!command_type_labels_.empty())
    {
        try
        {
            task_description = command_type_labels_.at(predicted_class_index);
        }
        catch (const std::out_of_range &)
        {
            std::cerr << "Error: Command type label not found for key: " << predicted_class_index << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: Command type labels are empty." << std::endl;
    }

    std::string entity_description = "None";
    if (!ner_results.empty())
    {
        // Find the index of the maximum value in the NER results
        auto max_ner_iter = std::max_element(ner_results.begin(), ner_results.end());
        int predicted_entity_index = 0;
        if (!ner_labels_.empty())
        {
            predicted_entity_index = std::distance(ner_results.begin(), max_ner_iter) % ner_labels_.size();
            try
            {
                entity_description = ner_labels_.at(predicted_entity_index);
            }
            catch (const std::out_of_range &)
            {
                std::cerr << "Error: NER label not found for key: " << predicted_entity_index << std::endl;
            }
        }
        else
        {
            std::cerr << "Error: NER labels are empty." << std::endl;
        }
    }

    return {task_description, entity_description};
}
