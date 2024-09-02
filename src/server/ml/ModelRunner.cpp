#include "ModelRunner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <nlohmann/json.hpp>

ModelRunner::ModelRunner(const std::string &model_path)
{
    model_ = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if (!model_)
    {
        throw std::runtime_error("Failed to load model: " + model_path);
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model_, resolver)(&interpreter_);
    if (!interpreter_)
    {
        throw std::runtime_error("Failed to build interpreter for model: " + model_path);
    }

    if (interpreter_->AllocateTensors() != kTfLiteOk)
    {
        throw std::runtime_error("Failed to allocate tensors for model: " + model_path);
    }
}

bool ModelRunner::IsLoaded() const
{
    return model_ != nullptr;
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

void ModelRunner::LoadLabels(const std::string &labels_path)
{
    std::ifstream labels_file(labels_path);
    if (!labels_file.is_open())
    {
        throw std::runtime_error("Failed to open labels file: " + labels_path);
    }

    nlohmann::json labels_json;
    labels_file >> labels_json;
    for (auto &[key, value] : labels_json.items())
    {
        int id = std::stoi(key);
        std::string label = value;
        labels_[id] = label;
    }
    labels_file.close();
    std::cout << "Loaded labels: " << labels_.size() << std::endl;
}

bool ModelRunner::RunInference(const std::string &input_text, std::vector<std::vector<float>> &result)
{
    if (!IsLoaded())
    {
        throw std::runtime_error("Model not loaded.");
    }

    std::vector<int> tokenized_input = TokenizeInput(input_text);

    TfLiteTensor *input_tensor = interpreter_->tensor(interpreter_->inputs()[0]);
    if (input_tensor == nullptr)
    {
        throw std::runtime_error("Failed to get input tensor");
    }
    std::memcpy(input_tensor->data.raw, tokenized_input.data(), tokenized_input.size() * sizeof(int));

    if (interpreter_->Invoke() != kTfLiteOk)
    {
        throw std::runtime_error("Failed to invoke TFLite interpreter");
    }

    TfLiteTensor *output_tensor = interpreter_->tensor(interpreter_->outputs()[0]);
    int output_size = output_tensor->dims->data[1];
    int num_classes = output_tensor->dims->data[2];
    result.resize(output_size, std::vector<float>(num_classes));
    for (int i = 0; i < output_size; ++i)
    {
        std::memcpy(result[i].data(), output_tensor->data.f + i * num_classes, num_classes * sizeof(float));
    }

    return true;
}

std::vector<int> ModelRunner::TokenizeInput(const std::string &input_text)
{
    std::vector<int> tokenized_input(max_length_, 0);
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

    if (tokenized_input.size() > max_length_)
    {
        throw std::length_error("Tokenized input size is too large");
    }

    return tokenized_input;
}

std::pair<std::string, std::vector<std::string>> ModelRunner::PredictlabelFromInput(const std::string &input)
{
    std::vector<std::vector<float>> results;

    std::cout << "Running inference for input: " << input << std::endl;

    // Run inference and check if it succeeded
    if (!RunInference(input, results))
    {
        throw std::runtime_error("Failed to run inference on input: " + input);
    }

    // Debug: Print the results size
    std::cout << "Inference results size: " << results.size() << " x "
              << (results.empty() ? 0 : results[0].size()) << std::endl;

    int predicted_class_index = std::distance(results[0].begin(), std::max_element(results[0].begin(), results[0].end()));
    std::string task_description = "Unknown";

    // Debug: Print the predicted class index and corresponding probability
    std::cout << "Predicted class index for task: " << predicted_class_index
              << " with probability: " << results[0][predicted_class_index] << std::endl;

    if (!labels_.empty())
    {
        try
        {
            task_description = labels_.at(predicted_class_index);
        }
        catch (const std::out_of_range &)
        {
            std::cerr << "Error: Label not found for key: " << predicted_class_index << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: Labels are empty." << std::endl;
    }

    std::vector<std::string> entity_descriptions;
    std::istringstream iss(input);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word)
    {
        words.push_back(word);
    }

    // Debug: Print the words and their corresponding inference result
    std::cout << "Words and their corresponding entity indices:" << std::endl;
    for (int i = 0; i < words.size(); ++i)
    {
        int predicted_entity_index = std::distance(results[i].begin(), std::max_element(results[i].begin(), results[i].end()));
        std::cout << "Word: " << words[i]
                  << " -> Predicted entity index: " << predicted_entity_index
                  << " with probability: " << results[i][predicted_entity_index] << std::endl;

        if (labels_[predicted_entity_index] != "O")
        {
            entity_descriptions.push_back(words[i] + " (" + labels_[predicted_entity_index] + ")");
        }
    }

    return {task_description, entity_descriptions};
}

std::string ModelRunner::ClassifySentence(const std::string &input)
{
    std::vector<std::vector<float>> results;

    // Run inference
    if (!RunInference(input, results))
    {
        throw std::runtime_error("Failed to run inference on input: " + input);
    }

    // Check if results are empty
    if (results.empty() || results[0].empty())
    {
        std::cerr << "Inference results are empty for input: " << input << std::endl;
        return "Unknown"; // Returning a default value or handling it as per your application logic
    }

    // Since this function is for classifying the whole sentence, we only need to consider the first output
    int predicted_class_index = std::distance(results[0].begin(), std::max_element(results[0].begin(), results[0].end()));

    // Retrieve the corresponding label for the predicted class
    std::string sentence_label = "Unknown";
    if (!labels_.empty())
    {
        try
        {
            sentence_label = labels_.at(predicted_class_index);
        }
        catch (const std::out_of_range &)
        {
            std::cerr << "Error: Label not found for key: " << predicted_class_index << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: Labels are empty." << std::endl;
    }

    return sentence_label;
}
