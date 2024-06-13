#include "TaskProcessor.h"
#include <iostream>

TaskProcessor::TaskProcessor(ModelRunner &runner, HomeAssistantAPI &homeAssistantAPI)
    : modelRunner_(runner), homeAssistantAPI_(homeAssistantAPI)
{
    // Initialize taskHandler_ with a valid function
    taskHandler_ = [this](const Task &task)
    {
        // Example task handling code
        std::cout << "Handling task: " << task.description << std::endl;
        // Additional task processing logic
    };
}

void TaskProcessor::processTask(const Task &task)
{
    // Ignore empty tasks
    if (task.description.empty())
    {
        std::cout << "Received an empty task, ignoring." << std::endl;
        return;
    }

    switch (task.type)
    {
    case Task::GENERAL:
        processGeneralTask(task);
        break;
    case Task::HOME_ASSISTANT:
        processHomeAssistantTask(task);
        break;
    case Task::ERROR:
        std::cerr << "Error task received: " << task.description << std::endl;
        break;
    }
}

void TaskProcessor::processGeneralTask(const Task &task)
{
    // General task processing logic here
    std::cout << "Processing general task: " << task.description << std::endl;
}

void TaskProcessor::processHomeAssistantTask(const Task &task)
{
    std::cout << "Processing Home Assistant task: " << task.description << std::endl;
    if (!task.service.empty())
    {
        homeAssistantAPI_.callService("homeassistant", task.service, task.entityId);
    }
    else if (!task.newState.empty())
    {
        homeAssistantAPI_.sendStateChange(task.entityId, task.newState);
    }
}
