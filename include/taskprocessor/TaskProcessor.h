#ifndef TASKPROCESSOR_H
#define TASKPROCESSOR_H

#include "Task.h"
#include <functional>
#include "HomeAssistantAPI.h"
#include "ModelRunner.h"


class TaskProcessor
{
public:
    TaskProcessor(HomeAssistantAPI *homeAssistantAPI, ModelRunner &nerModel, ModelRunner &classificationModel);
    void processTask(const Task &task);
private:
    std::function<void(const Task &task)> taskHandler_;
    bool processGeneralTask(const Task &task);
    bool processHomeAssistantTask(const Task &task);
    ModelRunner &nerModel_;
    ModelRunner &classificationModel_;
    HomeAssistantAPI *homeAssistantAPI_;
};

#endif