/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    06-06-2024
 * @Date updated    21-09-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the InputHandler class
 **/

#include "InputHandler.h"
#include <stdexcept>

void InputHandler::addTask(const Task &task)
{
    taskQueue.push(task);
}

bool InputHandler::hasTasks() const
{
    return !taskQueue.empty();
}

Task InputHandler::getNextTask()
{
    if (taskQueue.empty())
    {
        throw std::runtime_error("No tasks available");
    }
    Task task = taskQueue.front();
    taskQueue.pop();
    return task;
}
