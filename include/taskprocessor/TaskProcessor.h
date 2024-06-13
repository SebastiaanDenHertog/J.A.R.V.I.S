#ifndef TASK_PROCESSOR_H
#define TASK_PROCESSOR_H

#include "Task.h"
#include "model_runner.h"
#include <iostream>

class TaskProcessor
{
public:
    TaskProcessor(ModelRunner &runner);
    void processTask(const Task &task);

private:
    ModelRunner &modelRunner_;
    // Example function member
    std::function<void(const Task&)> taskHandler_;
};

#endif