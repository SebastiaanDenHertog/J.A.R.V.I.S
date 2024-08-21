#include "ModelRunner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
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

    // Load max_length_ if it exists in the tokenizer JSON
    if (tokenizer_json.contains("max_len"))
    {
        max_length_ = tokenizer_json["max_len"];
        std::cout << "Loaded max_length_ from tokenizer: " << max_length_ << std::endl;
    }
    else
    {
        throw std::runtime_error("max_len not found in tokenizer JSON");
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

bool ModelRunner::RunInference(const std::string &input_text, std::vector<std::vector<float>> &ner_result, std::vector<float> &command_type_result)
{
    std::string ner_model_name = "ner";
    std::string command_model_name = "command";

    if (!IsLoaded(ner_model_name) || !IsLoaded(command_model_name))
    {
        throw std::runtime_error("Model not loaded.");
    }

    // Tokenize the input text
    std::vector<int> tokenized_input = TokenizeInput(input_text);

    // Run NER model
    auto &ner_interpreter = interpreters_[ner_model_name];
    TfLiteTensor *ner_input_tensor = ner_interpreter->tensor(ner_interpreter->inputs()[0]);
    if (ner_input_tensor == nullptr)
    {
        throw std::runtime_error("Failed to get NER input tensor");
    }
    std::memcpy(ner_input_tensor->data.raw, tokenized_input.data(), tokenized_input.size() * sizeof(int));

    if (ner_interpreter->Invoke() != kTfLiteOk)
    {
        throw std::runtime_error("Failed to invoke NER TFLite interpreter");
    }

    TfLiteTensor *ner_output_tensor = ner_interpreter->tensor(ner_interpreter->outputs()[0]);
    int ner_output_size = ner_output_tensor->dims->data[1];
    int ner_num_classes = ner_output_tensor->dims->data[2];
    ner_result.resize(ner_output_size, std::vector<float>(ner_num_classes));
    for (int i = 0; i < ner_output_size; ++i)
    {
        std::memcpy(ner_result[i].data(), ner_output_tensor->data.f + i * ner_num_classes, ner_num_classes * sizeof(float));
    }

    // Run Command Type model
    auto &command_interpreter = interpreters_[command_model_name];
    TfLiteTensor *command_input_tensor = command_interpreter->tensor(command_interpreter->inputs()[0]);
    if (command_input_tensor == nullptr)
    {
        throw std::runtime_error("Failed to get Command Type input tensor");
    }
    std::memcpy(command_input_tensor->data.raw, tokenized_input.data(), tokenized_input.size() * sizeof(int));

    if (command_interpreter->Invoke() != kTfLiteOk)
    {
        throw std::runtime_error("Failed to invoke Command Type TFLite interpreter");
    }

    TfLiteTensor *command_output_tensor = command_interpreter->tensor(command_interpreter->outputs()[0]);
    command_type_result.resize(command_output_tensor->bytes / sizeof(float));
    std::memcpy(command_type_result.data(), command_output_tensor->data.raw, command_output_tensor->bytes);

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
        if (tokenizer_word_index_.find(word) != tokenizer_word_index_.end())
        {
            tokenized_input[index++] = tokenizer_word_index_[word];
        }
        else
        {
            tokenized_input[index++] = tokenizer_word_index_["<UNK>"];
        }
    }

    // Debugging statement to print tokenized input size
    std::cout << "Tokenized input size: " << tokenized_input.size() << std::endl;

    // Validation check
    if (tokenized_input.size() > max_length_)
    { // Adjust this value as needed
        throw std::length_error("Tokenized input size is too large");
    }

    return tokenized_input;
}

std::pair<std::string, std::vector<std::string>> ModelRunner::predictTaskFromInput(const std::string &input)
{
    std::vector<std::vector<float>> ner_results;
    std::vector<float> command_type_results;

    RunInference(input, ner_results, command_type_results);

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

    std::vector<std::string> entity_descriptions;
    if (!ner_results.empty())
    {
        std::istringstream iss(input);
        std::vector<std::string> words;
        std::string word;
        while (iss >> word)
        {
            words.push_back(word);
        }

        for (int i = 0; i < words.size(); ++i)
        {
            int predicted_entity_index = std::distance(ner_results[i].begin(), std::max_element(ner_results[i].begin(), ner_results[i].end()));
            if (ner_labels_[predicted_entity_index] != "O")
            {
                entity_descriptions.push_back(words[i] + " (" + ner_labels_[predicted_entity_index] + ")");
            }
        }
    }

    return {task_description, entity_descriptions};
}
