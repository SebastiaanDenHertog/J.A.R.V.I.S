#ifndef TASKPROCESSOR_H
#define TASKPROCESSOR_H

#include "Task.h"
#include "ModelRunner.h"
#include "HomeAssistantAPI.h"
#include <functional>

class TaskProcessor
{
public:
    TaskProcessor(ModelRunner &nerModel, ModelRunner &classificationModel, HomeAssistantAPI *homeAssistantAPI);

    void processTask(const Task &task);

private:
    ModelRunner &nerModel_; 
    ModelRunner &classificationModel_; 
    HomeAssistantAPI *homeAssistantAPI_;
    std::function<void(const Task &task)> taskHandler_;

    void processGeneralTask(const Task &task);
    void processHomeAssistantTask(const Task &task);
};

#endif
