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

    // Tokenize input
    std::vector<int> tokenized_input = TokenizeInput(input_text);

    TfLiteTensor *input_tensor = interpreter_->tensor(interpreter_->inputs()[0]);
    if (input_tensor == nullptr)
    {
        throw std::runtime_error("Failed to get input tensor");
    }

    // Determine the data type of the input tensor
    switch (input_tensor->type)
    {
    case kTfLiteInt32:
    {
        std::memcpy(input_tensor->data.raw, tokenized_input.data(), tokenized_input.size() * sizeof(int));
        break;
    }
    case kTfLiteFloat32:
    {
        // Convert tokenized_input to float
        std::vector<float> float_input(tokenized_input.begin(), tokenized_input.end());
        std::memcpy(input_tensor->data.f, float_input.data(), float_input.size() * sizeof(float));
        break;
    }
    // Add more cases if your models use different types
    default:
    {
        throw std::runtime_error("Unsupported input tensor type");
    }
    }

    // Invoke the interpreter
    if (interpreter_->Invoke() != kTfLiteOk)
    {
        throw std::runtime_error("Failed to invoke TFLite interpreter");
    }

    TfLiteTensor *output_tensor = interpreter_->tensor(interpreter_->outputs()[0]);
    if (output_tensor == nullptr)
    {
        throw std::runtime_error("Failed to get output tensor");
    }

    // Debug: Print output tensor information
    std::cout << "Output Tensor Type: " << output_tensor->type << std::endl;
    std::cout << "Output Tensor Dimensions: ";
    for (int i = 0; i < output_tensor->dims->size; ++i)
    {
        std::cout << output_tensor->dims->data[i] << " ";
    }
    std::cout << std::endl;

    // Handle different output tensor shapes based on the model type
    if (output_tensor->type == kTfLiteFloat32)
    {
        if (output_tensor->dims->size == 2)
        {
            int batch_size = output_tensor->dims->data[0];
            int num_classes = output_tensor->dims->data[1];
            result.resize(batch_size, std::vector<float>(num_classes));
            for (int i = 0; i < batch_size; ++i)
            {
                std::memcpy(result[i].data(), output_tensor->data.f + i * num_classes, num_classes * sizeof(float));
            }
        }
        else if (output_tensor->dims->size == 3)
        {
            // Assuming NER model output: [batch_size, sequence_length, num_entities]
            int batch_size = output_tensor->dims->data[0];
            int sequence_length = output_tensor->dims->data[1];
            int num_entities = output_tensor->dims->data[2];
            result.resize(sequence_length, std::vector<float>(num_entities));
            for (int i = 0; i < sequence_length; ++i)
            {
                std::memcpy(result[i].data(), output_tensor->data.f + i * num_entities, num_entities * sizeof(float));
            }
        }
        else
        {
            throw std::runtime_error("Unexpected output tensor dimensions");
        }
    }
    else
    {
        throw std::runtime_error("Unsupported output tensor type");
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

    // Run inference on the input text to get NER predictions
    if (!RunInference(input, results))
    {
        throw std::runtime_error("Failed to run inference on input: " + input);
    }

    std::string task_description = "Entities Extracted";
    std::vector<std::string> entity_descriptions;

    // Tokenize the input text into words
    std::istringstream iss(input);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word)
    {
        words.push_back(word);
    }

    // Check if the number of words matches the result size (predicted entities)
    if (words.size() > results.size())
    {
        std::cerr << "Warning: Fewer predictions than words in the input text." << std::endl;
    }

    // Map each word to its predicted NER label
    for (int i = 0; i < words.size(); ++i)
    {
        // Ensure we do not exceed the number of predictions
        if (i >= results.size())
            break;

        // Find the predicted entity index with the highest probability
        int predicted_entity_index = std::distance(results[i].begin(), std::max_element(results[i].begin(), results[i].end()));
        float predicted_probability = results[i][predicted_entity_index];

        // Debug: Print the word and the predicted entity
        std::cout << "Word: " << words[i]
                  << " -> Predicted entity index: " << predicted_entity_index
                  << " with probability: " << predicted_probability << std::endl;

        // Only add the predicted label if the confidence is high enough (e.g., > 0.5)
        if (predicted_probability > 0.5 && labels_.find(predicted_entity_index) != labels_.end())
        {
            std::string predicted_label = labels_[predicted_entity_index];
            entity_descriptions.push_back(words[i] + " (" + predicted_label + ")");
        }
        else
        {
            // If confidence is too low or the label is unknown, mark as "O" (outside entity)
            entity_descriptions.push_back(words[i] + " (O)");
        }
    }

    return {task_description, entity_descriptions};
}

std::string ModelRunner::ClassifySentence(const std::string &input)
{
    std::vector<std::vector<float>> results;

    // Run inference on the input text to get classification predictions
    if (!RunInference(input, results))
    {
        throw std::runtime_error("Failed to run inference on input: " + input);
    }

    // Check if results are empty
    if (results.empty() || results[0].empty())
    {
        std::cerr << "Inference results are empty for input: " << input << std::endl;
        return "Unknown";
    }

    // The results should contain a vector of probabilities for each class
    const std::vector<float> &class_probabilities = results[0];

    // Find the class with the highest probability
    int predicted_class_index = std::distance(class_probabilities.begin(),
                                              std::max_element(class_probabilities.begin(),
                                                               class_probabilities.end()));
    float predicted_probability = class_probabilities[predicted_class_index];

    // Debug: Print the predicted class and probability
    std::cout << "Predicted class index: " << predicted_class_index
              << " with probability: " << predicted_probability << std::endl;

    // Check if the predicted probability is below a threshold (e.g., 0.85)
    if (predicted_probability < 0.85)
    {
        return "Info"; // Return "Info" if the confidence is below the threshold
    }

    // Otherwise, return the label corresponding to the predicted class
    std::string sentence_label = "Unknown";
    if (!labels_.empty())
    {
        try
        {
            sentence_label = labels_.at(predicted_class_index);
            std::cout << "Mapped Label: " << sentence_label << std::endl;
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
