#include "TaskProcessor.h"
#include <iostream>

TaskProcessor::TaskProcessor(ModelRunner &model_runner) : model_runner_(model_runner) {}

std::function<void(const Task &)> TaskProcessor::getTaskHandler(const std::string &type)
{
    // Map task types to handler functions
    std::map<std::string, std::function<void(const Task &)>> handlers = {
        {"email", [this](const Task &task)
         { this->handleEmailTask(task); }},
        {"reminder", [this](const Task &task)
         { this->handleReminderTask(task); }},
        {"general", [this](const Task &task)
         { this->handleGeneralTask(task); }}};

    return handlers[type];
}

void TaskProcessor::processTask(const Task &task)
{
    auto handler = getTaskHandler(task.type);
    handler(task);
}

void TaskProcessor::handleEmailTask(const Task &task)
{
    std::cout << "Processing email task: " << task.description << std::endl;
}

void TaskProcessor::handleReminderTask(const Task &task)
{
    std::cout << "Processing reminder task: " << task.description << std::endl;
}

void TaskProcessor::handleGeneralTask(const Task &task)
{
    std::cout << "Processing general task: " << task.description << std::endl;
}
