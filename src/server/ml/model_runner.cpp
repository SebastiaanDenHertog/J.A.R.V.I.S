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
        // Initialize with some basic tokens if needed
        token_map_["<PAD>"] = 0;
        token_map_["<UNK>"] = 1;
        next_id_ = 2;
    }

    std::vector<int> Tokenize(const std::string &input)
    {
        std::vector<int> tokens;
        std::istringstream stream(input);
        std::string word;
        while (stream >> word)
        {
            tokens.push_back(GetTokenId(word));
        }
        return tokens;
    }

    void EnsureSize(std::vector<int> &tokens, size_t expected_size)
    {
        if (tokens.size() < expected_size)
        {
            tokens.resize(expected_size, token_map_["<PAD>"]); // Pad with PAD token
        }
        else if (tokens.size() > expected_size)
        {
            tokens.resize(expected_size); // Truncate if needed
        }
    }

private:
    int GetTokenId(const std::string &word)
    {
        if (token_map_.find(word) == token_map_.end())
        {
            token_map_[word] = next_id_++;
        }
        return token_map_[word];
    }

    std::unordered_map<std::string, int> token_map_;
    int next_id_;
};

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

bool ModelRunner::RunWakeUpModel(const std::string &input, float &result)
{
    if (!IsLoaded("wake_up"))
    {
        std::cerr << "Wake up model is not loaded properly." << std::endl;
        return false;
    }
    auto &interpreter = interpreters_.at("wake_up");
    float *input_tensor = interpreter->typed_input_tensor<float>(0);
    std::memcpy(input_tensor, input.data(), input.size() * sizeof(float));

    if (interpreter->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke wake up model!" << std::endl;
        return false;
    }

    result = *interpreter->typed_output_tensor<float>(0);
    return true;
}

bool ModelRunner::RunNLPModel(const std::string &input, std::vector<float> &result)
{
    if (!IsLoaded("nlp"))
    {
        std::cerr << "NLP model is not loaded properly." << std::endl;
        return false;
    }
    auto &interpreter = interpreters_.at("nlp");

    // Tokenize and preprocess the input
    Tokenizer tokenizer;
    std::vector<int> tokenized_input = tokenizer.Tokenize(input);
    TfLiteTensor *input_tensor = interpreter->tensor(interpreter->inputs()[0]);
    size_t expected_size = input_tensor->bytes / sizeof(int);
    tokenizer.EnsureSize(tokenized_input, expected_size);

    // Ensure the input tensor has the correct size
    if (input_tensor->bytes != tokenized_input.size() * sizeof(int))
    {
        std::cerr << "Input tensor size mismatch! Expected " << input_tensor->bytes << " bytes but got " << tokenized_input.size() * sizeof(int) << " bytes." << std::endl;
        return false;
    }

    // Use safer copying method
    int *input_data = interpreter->typed_input_tensor<int>(0);
    for (size_t i = 0; i < tokenized_input.size(); ++i)
    {
        input_data[i] = tokenized_input[i];
    }

    // Log details for debugging
    std::cout << "Input Tensor Size: " << input_tensor->bytes << " bytes" << std::endl;
    std::cout << "Tokenized Input Size: " << tokenized_input.size() * sizeof(int) << " bytes" << std::endl;

    // Ensure correct tensor data type
    if (input_tensor->type != kTfLiteInt32)
    {
        std::cerr << "Input tensor type mismatch! Expected kTfLiteInt32 but got " << input_tensor->type << std::endl;
        return false;
    }

    // Log input tensor details
    std::cout << "Input Tensor Details: " << std::endl;
    std::cout << "  Type: " << input_tensor->type << std::endl;
    std::cout << "  Dimensions: " << input_tensor->dims->size << std::endl;
    for (int i = 0; i < input_tensor->dims->size; ++i)
    {
        std::cout << "    Dim " << i << ": " << input_tensor->dims->data[i] << std::endl;
    }

    // Check the data to be copied
    std::cout << "Tokenized input data: ";
    for (const auto &token : tokenized_input)
    {
        std::cout << token << " ";
    }
    std::cout << std::endl;

    // Check memory boundaries before copying
    std::cout << "Input tensor data address: " << static_cast<void *>(input_tensor->data.raw) << std::endl;
    std::cout << "Tokenized input data address: " << static_cast<void *>(tokenized_input.data()) << std::endl;

    // Additional check before invoking
    if (interpreter->inputs().size() == 0 || interpreter->outputs().size() == 0)
    {
        std::cerr << "Interpreter inputs/outputs are not set correctly." << std::endl;
        return false;
    }

    if (interpreter->Invoke() != kTfLiteOk)
    {
        std::cerr << "Failed to invoke NLP model!" << std::endl;
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

    // Check if the output tensor type matches the expected type
    if (output_tensor->type == kTfLiteFloat32)
    {
        const float *output = interpreter->typed_output_tensor<float>(0);
        if (output == nullptr)
        {
            std::cerr << "Typed output tensor is null." << std::endl;
            return false;
        }
        result.assign(output, output + output_tensor->dims->data[output_tensor->dims->size - 1]);
    }
    else if (output_tensor->type == kTfLiteUInt8)
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
            result[i] = static_cast<float>(output[i]); // Convert directly to float
        }
    }
    else
    {
        std::cerr << "Unsupported output tensor type: " << output_tensor->type << std::endl;
        return false;
    }

    return true;
}

bool ModelRunner::RunLLMModel(const std::vector<float> &input, std::string &result)
{
    if (!IsLoaded("llm"))
    {
        std::cerr << "LLM model is not loaded properly." << std::endl;
        return false;
    }
    auto &interpreter = interpreters_.at("llm");

    // Ensure the input tensor has the correct size
    TfLiteTensor *input_tensor = interpreter->tensor(interpreter->inputs()[0]);
    if (input_tensor->bytes != input.size() * sizeof(float))
    {
        std::cerr << "Input tensor size mismatch! Expected " << input_tensor->bytes << " bytes but got " << input.size() * sizeof(float) << " bytes." << std::endl;
        return false;
    }

    std::memcpy(input_tensor->data.raw, input.data(), input_tensor->bytes);

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

    if (output_tensor->type == kTfLiteFloat32)
    {
        const float *output = interpreter->typed_output_tensor<float>(0);
        result.assign(reinterpret_cast<const char *>(output), interpreter->tensor(interpreter->outputs()[0])->bytes);
    }
    else if (output_tensor->type == kTfLiteUInt8)
    {
        const uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);
        std::vector<uint8_t> output_vector(output, output + output_tensor->dims->data[output_tensor->dims->size - 1]);
        result.assign(output_vector.begin(), output_vector.end());
    }
    else
    {
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
    float wake_up_result;
    if (!RunWakeUpModel(input, wake_up_result))
    {
        std::cerr << "Failed to run wake up model." << std::endl;
        return Task("", 0, "error");
    }

    if (wake_up_result < 0.5)
    {
        std::cerr << "Wake up model did not detect activation." << std::endl;
        return Task("", 0, "no_wake_up");
    }

    std::vector<float> nlp_result;
    if (!RunNLPModel(input, nlp_result))
    {
        std::cerr << "Failed to run NLP model." << std::endl;
        return Task("", 0, "error");
    }

    std::string llm_result;
    if (!RunLLMModel(nlp_result, llm_result))
    {
        std::cerr << "Failed to run LLM model." << std::endl;
        return Task("", 0, "error");
    }

    return Task(llm_result, 1, "predicted_task");
}
