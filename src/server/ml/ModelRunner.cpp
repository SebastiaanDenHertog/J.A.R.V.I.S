/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    13-06-2024
 * @Date updated    17-09-2025
 * @Description     Constructor, destructor and methods for the ModelRunner class
 *                  (TensorFlow SavedModel C++ backend)
 **/

#include "ModelRunner.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <algorithm>

#include <nlohmann/json.hpp>

// TensorFlow
#include <tensorflow/cc/saved_model/loader.h>
#include <tensorflow/cc/saved_model/tag_constants.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/framework/tensor_shape.h>
#include <tensorflow/core/lib/core/status.h>
#include <tensorflow/core/platform/env.h>

using tensorflow::Status;
using tensorflow::Tensor;
using tensorflow::TensorShape;
using tensorflow::DT_FLOAT;
using tensorflow::DT_INT32;

ModelRunner::ModelRunner(const std::string& model_dir,
                         const std::string& input_op,
                         const std::string& output_op)
    : input_op_(input_op),
      output_op_(output_op)
{
    tensorflow::SessionOptions session_opts;
    tensorflow::RunOptions run_opts;

    tensorflow::Status status = tensorflow::LoadSavedModel(
        session_opts, run_opts, model_dir,
        {tensorflow::kSavedModelTagServe}, &bundle_);

    if (!status.ok()) {
        throw std::runtime_error(
            "Failed to load SavedModel from '" + model_dir + "': " + status.ToString());
    }

    session_ = bundle_.GetSession();
    loaded_ = (session_ != nullptr);
    if (!loaded_) {
        throw std::runtime_error("SavedModel loaded but session is null.");
    }
}

bool ModelRunner::IsLoaded() const
{
    return loaded_;
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
    if (max_length_ <= 0)
    {
        throw std::runtime_error("Tokenizer not loaded or max_length_ invalid.");
    }

    // Tokenize input (padded/truncated to max_length_)
    std::vector<int> tokenized_input = TokenizeInput(input_text);

    // Prepare feed dict; we’ll try int32 first (common for token IDs), and if that
    // fails due to dtype mismatch, we’ll retry with float32.
    auto build_int_tensor = [&]() -> Tensor {
        Tensor t(DT_INT32, TensorShape({1, max_length_}));
        auto flat = t.flat<int32_t>();
        for (int i = 0; i < max_length_; ++i) {
            flat(i) = (i < static_cast<int>(tokenized_input.size())) ? tokenized_input[i] : 0;
        }
        return t;
    };

    auto build_float_tensor = [&]() -> Tensor {
        Tensor t(DT_FLOAT, TensorShape({1, max_length_}));
        auto flat = t.flat<float>();
        for (int i = 0; i < max_length_; ++i) {
            flat(i) = (i < static_cast<int>(tokenized_input.size()))
                        ? static_cast<float>(tokenized_input[i]) : 0.0f;
        }
        return t;
    };

    std::vector<std::pair<std::string, Tensor>> feed_dict;
    std::vector<Tensor> outputs;

    // First attempt: int32
    feed_dict.clear();
    outputs.clear();
    feed_dict.emplace_back(input_op_, build_int_tensor());
    Status status = session_->Run(feed_dict, {output_op_}, {}, &outputs);

    // Retry with float32 if we hit a type error or placeholder dtype mismatch
    if (!status.ok()) {
        bool maybe_dtype_issue =
            status.message().find("type") != std::string::npos ||
            status.message().find("dtype") != std::string::npos ||
            status.message().find("cannot be cast") != std::string::npos ||
            status.message().find("Expected") != std::string::npos;

        if (maybe_dtype_issue) {
            feed_dict.clear();
            outputs.clear();
            feed_dict.emplace_back(input_op_, build_float_tensor());
            status = session_->Run(feed_dict, {output_op_}, {}, &outputs);
        }
    }

    if (!status.ok()) {
        throw std::runtime_error("Failed to run TF session: " + status.ToString());
    }

    if (outputs.empty()) {
        throw std::runtime_error("No outputs returned from session run.");
    }

    const Tensor& out = outputs[0];

    // Expect float outputs for probabilities/logits
    if (out.dtype() != DT_FLOAT) {
        throw std::runtime_error("Unsupported output tensor dtype. Expected float32.");
    }

    // Handle shapes:
    // - Classification: [1, num_classes]
    // - Token classification (NER): [1, sequence_length, num_entities]
    const auto& shape = out.shape();
    if (shape.dims() == 2) {
        // [1, num_classes]
        if (shape.dim_size(0) != 1) {
            throw std::runtime_error("Unexpected batch size for 2D output (expected 1).");
        }
        const int num_classes = static_cast<int>(shape.dim_size(1));
        result.clear();
        result.resize(1, std::vector<float>(num_classes, 0.0f));

        auto flat = out.flat<float>(); // length == num_classes
        std::memcpy(result[0].data(), flat.data(), sizeof(float) * num_classes);
    }
    else if (shape.dims() == 3) {
        // [1, sequence_length, num_entities]
        if (shape.dim_size(0) != 1) {
            throw std::runtime_error("Unexpected batch size for 3D output (expected 1).");
        }
        const int sequence_length = static_cast<int>(shape.dim_size(1));
        const int num_entities    = static_cast<int>(shape.dim_size(2));
        result.clear();
        result.resize(sequence_length, std::vector<float>(num_entities, 0.0f));

        // Copy row by row from the underlying buffer
        auto t3 = out.tensor<float, 3>(); // [1, L, C]
        for (int i = 0; i < sequence_length; ++i) {
            for (int c = 0; c < num_entities; ++c) {
                result[i][c] = t3(0, i, c);
            }
        }
    }
    else {
        std::ostringstream oss;
        oss << "Unexpected output tensor rank " << shape.dims()
            << ". Supported: [1, C] or [1, L, C].";
        throw std::runtime_error(oss.str());
    }

    return true;
}

std::vector<int> ModelRunner::TokenizeInput(const std::string &input_text)
{
    std::vector<int> tokenized_input;
    tokenized_input.reserve(max_length_);

    std::istringstream iss(input_text);
    std::string word;

    while (iss >> word && static_cast<int>(tokenized_input.size()) < max_length_)
    {
        auto it = tokenizer_word_index_.find(word);
        if (it != tokenizer_word_index_.end())
        {
            tokenized_input.push_back(it->second);
        }
        else
        {
            // Use <UNK> if present; else 0
            auto unk = tokenizer_word_index_.find("<UNK>");
            tokenized_input.push_back(unk != tokenizer_word_index_.end() ? unk->second : 0);
        }
    }

    // Pad with zeros up to max_length_
    while (static_cast<int>(tokenized_input.size()) < max_length_) {
        tokenized_input.push_back(0);
    }

    if (static_cast<int>(tokenized_input.size()) > max_length_)
    {
        throw std::length_error("Tokenized input size is too large");
    }

    return tokenized_input;
}

std::pair<std::string, std::vector<std::string>> ModelRunner::PredictlabelFromInput(const std::string &input)
{
    std::vector<std::vector<float>> results;

    if (!RunInference(input, results))
    {
        throw std::runtime_error("Failed to run inference on input: " + input);
    }

    std::string task_description = "Entities Extracted";
    std::vector<std::string> entity_descriptions;

    std::istringstream iss(input);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word)
    {
        words.push_back(word);
    }
    if (words.size() > results.size())
    {
        std::cerr << "Warning: Fewer predictions than words in the input text." << std::endl;
    }
    for (int i = 0; i < static_cast<int>(words.size()); ++i)
    {
        if (i >= static_cast<int>(results.size()))
            break;
        int predicted_entity_index = std::distance(results[i].begin(), std::max_element(results[i].begin(), results[i].end()));
        float predicted_probability = results[i][predicted_entity_index];
        std::cout << "Word: " << words[i]
                  << " -> Predicted entity index: " << predicted_entity_index
                  << " with probability: " << predicted_probability << std::endl;
        if (predicted_probability > 0.5f && labels_.find(predicted_entity_index) != labels_.end())
        {
            std::string predicted_label = labels_[predicted_entity_index];
            entity_descriptions.push_back(words[i] + " (" + predicted_label + ")");
        }
        else
        {
            entity_descriptions.push_back(words[i] + " (O)");
        }
    }
    return {task_description, entity_descriptions};
}