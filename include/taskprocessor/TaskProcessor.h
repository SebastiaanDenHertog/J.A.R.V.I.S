#ifndef TASKPROCESSOR_H
#define TASKPROCESSOR_H

#include "Task.h"
#include "model_runner.h"
#include <functional>
#include <map>
#include <string>

class TaskProcessor
{
public:
    TaskProcessor(ModelRunner &model_runner);
    std::function<void(const Task &)> getTaskHandler(const std::string &type);
    void processTask(const Task &task);

private:
    void handleEmailTask(const Task &task);
    void handleReminderTask(const Task &task);
    void handleGeneralTask(const Task &task);

    ModelRunner &model_runner_;
};

#endif // TASKPROCESSOR_H
