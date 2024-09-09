#ifndef TASKPROCESSOR_H
#define TASKPROCESSOR_H

#include "Task.h"
#include "HomeAssistantAPI.h"
#include <functional>

class TaskProcessor
{
public:
    TaskProcessor(HomeAssistantAPI *homeAssistantAPI);

    void processTask(const Task &task);

private:
    HomeAssistantAPI *homeAssistantAPI_;
    std::function<void(const Task &task)> taskHandler_;

    void processGeneralTask(const Task &task);
    void processHomeAssistantTask(const Task &task);
};

#endif

#if defined(BUILD_SERVER) || defined(BUILD_FULL)

#include "ModelRunner.h"

class TaskProcessor
{
public:
    addModels(ModelRunner &nerModel, ModelRunner &classificationModel, HomeAssistantAPI *homeAssistantAPI);

private:
    ModelRunner &nerModel_;
    ModelRunner &classificationModel_;
};

#endif