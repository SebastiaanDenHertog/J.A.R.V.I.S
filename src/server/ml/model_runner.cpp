#include "model_runner.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/tools/logging.h"
#include "tensorflow/lite/delegates/xnnpack/xnnpack_delegate.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <sstream>

// Tokenizer class to convert words to token IDs
class Tokenizer
{
public:
    Tokenizer()
    {
        word_index_["<OOV>"] = 1;
    }

    void fit_on_texts(const std::vector<std::string> &texts)
    {
        int index = 2; // Start indexing from 2 as 1 is for OOV
        for (const auto &text : texts)
        {
            std::istringstream iss(text);
            std::string word;
            while (iss >> word)
            {
                if (word_index_.find(word) == word_index_.end())
                {
                    word_index_[word] = index++;
                }
            }
        }
    }

    std::vector<int> texts_to_sequences(const std::string &text)
    {
        std::vector<int> sequence;
        std::istringstream iss(text);
        std::string word;
        while (iss >> word)
        {
            if (word_index_.find(word) != word_index_.end())
            {
                sequence.push_back(word_index_[word]);
            }
            else
            {
                sequence.push_back(word_index_["<OOV>"]);
            }
        }
        std::cerr << "Text: " << text << " -> Sequence: ";
        for (const auto &val : sequence)
        {
            std::cerr << val << " ";
        }
        std::cerr << std::endl;
        return sequence;
    }

private:
    std::unordered_map<std::string, int> word_index_;
};

// Utility function to prepare input
std::vector<float> prepare_input(const std::string &text, Tokenizer &tokenizer, size_t sequence_length)
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

    std::vector<float> input(sequence.begin(), sequence.end());
    return input;
}

bool ModelRunner::LoadModel(const std::string &model_path, const std::string &model_name)
{
    auto model = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if (!model)
    {
        std::cerr << "Failed to load model: " << model_name << std::endl;
        return false;
    }
    models_[model_name] = std::move(model);
    return CreateInterpreter(model_name);
}

bool ModelRunner::CreateInterpreter(const std::string &model_name)
{
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*models_[model_name], resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    if (builder(&interpreter) != kTfLiteOk)
    {
        std::cerr << "Failed to build interpreter for model: " << model_name << std::endl;
        return false;
    }

    // Configure XNNPACK delegate
    TfLiteXNNPackDelegateOptions options = TfLiteXNNPackDelegateOptionsDefault();
    options.num_threads = 4; // Adjust the number of threads based on your needs
    auto *delegate = TfLiteXNNPackDelegateCreate(&options);

    if (interpreter->ModifyGraphWithDelegate(delegate) != kTfLiteOk)
    {
        std::cerr << "Failed to apply XNNPACK delegate for model: " << model_name << std::endl;
        return false;
    }

    if (interpreter->AllocateTensors() != kTfLiteOk)
    {
        std::cerr << "Failed to allocate tensors for model: " << model_name << std::endl;
        return false;
    }
    interpreters_[model_name] = std::move(interpreter);
    return true;
}

ModelRunner::ModelRunner(const std::unordered_map<std::string, std::string> &model_paths)
{
    for (const auto &model_path : model_paths)
    {
        if (!LoadModel(model_path.second, model_path.first))
        {
            std::cerr << "Failed to load model: " << model_path.first << std::endl;
        }
    }
}

bool ModelRunner::IsLoaded(const std::string &model_name) const
{
    return models_.find(model_name) != models_.end() && interpreters_.find(model_name) != interpreters_.end();
}

bool ModelRunner::RunNLPModel(const std::string &input, std::vector<float> &result)
{
    if (!IsLoaded("nlp"))
    {
        std::cerr << "NLP model is not loaded properly." << std::endl;
        return false;
    }
    auto &interpreter = interpreters_.at("nlp");
    if (interpreter == nullptr)
    {
        std::cerr << "Interpreter is null." << std::endl;
        return false;
    }

    // Tokenize and preprocess the input
    Tokenizer tokenizer;
    std::vector<std::string> texts = {input}; // Replace with actual text corpus
    tokenizer.fit_on_texts(texts);
    std::vector<int> tokenized_input = tokenizer.texts_to_sequences(input);
    if (tokenized_input.size() < 20)
    {
        tokenized_input.resize(20, 0); // Padding with 0s
    }
    else if (tokenized_input.size() > 20)
    {
        tokenized_input.resize(20); // Truncate if needed
    }

    // Set the input tensor
    int *input_data = interpreter->typed_input_tensor<int>(0);
    if (input_data == nullptr)
    {
        std::cerr << "Input tensor data is null." << std::endl;
        return false;
    }
    std::memcpy(input_data, tokenized_input.data(), tokenized_input.size() * sizeof(int));

    // Debug: Print input data
    std::cerr << "Input tensor values: ";
    for (const auto &val : tokenized_input)
    {
        std::cerr << val << " ";
    }
    std::cerr << std::endl;

    // Prepare additional inputs (dummy data as an example)
    std::vector<float> additional_features = {0.0, 1.0, 0.0, 1.0};   // Example features
    std::vector<int> entity_input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; // Example entity input

    // Set additional input tensors
    if (interpreter->inputs().size() > 1)
    {
        float *feature_input_data = interpreter->typed_input_tensor<float>(1);
        if (feature_input_data != nullptr)
        {
            std::memcpy(feature_input_data, additional_features.data(), additional_features.size() * sizeof(float));
        }
    }

    if (interpreter->inputs().size() > 2)
    {
        int *entity_input_data = interpreter->typed_input_tensor<int>(2);
        if (entity_input_data != nullptr)
        {
            std::memcpy(entity_input_data, entity_input.data(), entity_input.size() * sizeof(int));
        }
    }

    // Invoke the model
    if (interpreter->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke NLP model!" << std::endl;
        return false;
    }

    // Handle different output tensor types and extract detailed results
    const TfLiteTensor *output_tensor = interpreter->tensor(interpreter->outputs()[0]);
    if (output_tensor == nullptr)
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

    switch (output_tensor->type)
    {
    case kTfLiteFloat32:
    {
        const float *output = interpreter->typed_output_tensor<float>(0);
        if (output == nullptr)
        {
            std::cerr << "Typed output tensor is null." << std::endl;
            return false;
        }
        result.assign(output, output + output_tensor->dims->data[output_tensor->dims->size - 1]);
        break;
    }
    case kTfLiteUInt8:
    {
        const uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);
        if (output == nullptr)
        {
            std::cerr << "Typed output tensor is null." << std::endl;
            return false;
        }
        result.resize(output_tensor->dims->data[output_tensor->dims->size - 1]);
        for (size_t i = 0; i < result.size(); ++i)
        {
            result[i] = static_cast<float>(output[i]); // Convert to float
        }
        break;
    }
    case kTfLiteInt64:
    {
        const int64_t *output = interpreter->typed_output_tensor<int64_t>(0);
        if (output == nullptr)
        {
            std::cerr << "Typed output tensor is null." << std::endl;
            return false;
        }
        result.resize(output_tensor->dims->data[output_tensor->dims->size - 1]);
        for (size_t i = 0; i < result.size(); ++i)
        {
            result[i] = static_cast<float>(output[i]); // Convert to float
        }
        break;
    }
    case kTfLiteInt32:
    {
        const int32_t *output = interpreter->typed_output_tensor<int32_t>(0);
        if (output == nullptr)
        {
            std::cerr << "Typed output tensor is null." << std::endl;
            return false;
        }
        result.resize(output_tensor->dims->data[output_tensor->dims->size - 1]);
        for (size_t i = 0; i < result.size(); ++i)
        {
            result[i] = static_cast<float>(output[i]); // Convert to float
        }
        break;
    }
    default:
        std::cerr << "Unsupported output tensor type: " << output_tensor->type << std::endl;
        return false;
    }

    // Print detailed results for debugging
    std::cerr << "Model output: ";
    for (const auto &val : result)
    {
        std::cerr << val << " ";
    }
    std::cerr << std::endl;

    return true;
}

bool ModelRunner::RunLLMModel(const std::string &input_text, std::string &result)
{
    if (!IsLoaded("llm"))
    {
        std::cerr << "LLM model is not loaded properly." << std::endl;
        return false;
    }

    // Tokenize and prepare input
    Tokenizer tokenizer;
    std::vector<std::string> texts = {"sample text 1", "sample text 2"}; // Replace with actual text corpus
    tokenizer.fit_on_texts(texts);
    std::vector<float> input = prepare_input(input_text, tokenizer, 100);

    auto &interpreter = interpreters_.at("llm");

    // Ensure the input tensor has the correct size
    TfLiteTensor *input_tensor = interpreter->tensor(interpreter->inputs()[0]);
    size_t expected_size = input_tensor->bytes / sizeof(float);
    if (input.size() != expected_size)
    {
        std::cerr << "Input tensor size mismatch! Expected " << expected_size << " elements but got " << input.size() << " elements." << std::endl;
        return false;
    }

    // Copy the input data to the input tensor
    std::memcpy(input_tensor->data.raw, input.data(), input_tensor->bytes);

    // Additional debugging information
    std::cerr << "Running LLM model with input size: " << input.size() << std::endl;

    // Debugging: print the input values
    std::cerr << "Input values: ";
    for (const auto &val : input)
    {
        std::cerr << val << " ";
    }
    std::cerr << std::endl;

    // Additional validation step: print the input tensor details
    std::cerr << "Input tensor details:" << std::endl;
    std::cerr << "  Tensor type: " << input_tensor->type << std::endl;
    std::cerr << "  Dimensions: " << input_tensor->dims->size << std::endl;
    for (int i = 0; i < input_tensor->dims->size; ++i)
    {
        std::cerr << "    Dim " << i << ": " << input_tensor->dims->data[i] << std::endl;
    }

    // Invoke the model
    if (interpreter->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke LLM model!" << std::endl;
        return false;
    }

    const TfLiteTensor *output_tensor = interpreter->tensor(interpreter->outputs()[0]);
    if (output_tensor == nullptr)
    {
        std::cerr << "Output tensor is null." << std::endl;
        return false;
    }

    // Log output tensor details
    std::cout << "Output Tensor Details: " << std::endl;
    std::cout << "  Type: " << output_tensor->type << std::endl;
    std::cout << "  Dimensions: " << output_tensor->dims->size << std::endl;
    for (int i = 0; i < output_tensor->dims->size; ++i)
    {
        std::cout << "    Dim " << i << ": " << output_tensor->dims->data[i] << std::endl;
    }

    // Handle different output tensor types
    switch (output_tensor->type)
    {
    case kTfLiteFloat32:
    {
        const float *output = interpreter->typed_output_tensor<float>(0);
        size_t output_size = output_tensor->bytes / sizeof(float);
        result.assign(reinterpret_cast<const char *>(output), output_size * sizeof(float));
        break;
    }
    case kTfLiteUInt8:
    {
        const uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);
        std::vector<uint8_t> output_vector(output, output + output_tensor->dims->data[output_tensor->dims->size - 1]);
        result.assign(output_vector.begin(), output_vector.end());
        break;
    }
    default:
        std::cerr << "Unsupported output tensor type for LLM model: " << output_tensor->type << std::endl;
        return false;
    }

    return true;
}

void ModelRunner::StoreUserContext(const std::string &context)
{
    user_context = context;
}

std::string ModelRunner::GetUserContext() const
{
    return user_context;
}

Task ModelRunner::predictTaskFromInput(const std::string &input)
{
    std::vector<float> nlp_result;
    if (!RunNLPModel(input, nlp_result))
    {
        std::cerr << "Failed to run NLP model." << std::endl;
        return Task("", 0, "error");
    }

    std::string llm_result;
    if (!RunLLMModel(input, llm_result))
    {
        std::cerr << "Failed to run LLM model." << std::endl;
        return Task("", 0, "error");
    }

    return Task(llm_result, 1, "predicted_task");
}
