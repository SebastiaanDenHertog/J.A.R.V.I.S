/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    06-06-2024
 * @Date updated    13-06-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the InputHandler class
 **/

#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <queue>
#include "Task.h"

class InputHandler
{
public:
    void addTask(const Task &task);
    bool hasTasks() const;
    Task getNextTask();

private:
    std::queue<Task> taskQueue;
};

#endif // INPUTHANDLER_H
