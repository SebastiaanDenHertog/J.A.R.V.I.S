#include "TaskProcessor.h"

TaskProcessor::TaskProcessor(ModelRunner &runner)
    : modelRunner_(runner)
{
    // Initialize taskHandler_ with a valid function
    taskHandler_ = [this](const Task &task) {
        // Example task handling code
        std::cout << "Handling task: " << task.description << std::endl;
        // Additional task processing logic
    };
}

void TaskProcessor::processTask(const Task &task)
{
    if (taskHandler_)
    {
        taskHandler_(task);
    }
    else
    {
        std::cerr << "Task handler is not set!" << std::endl;
    }
}