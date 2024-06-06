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
