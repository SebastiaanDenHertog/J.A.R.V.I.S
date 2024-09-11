#ifndef TASKPROCESSOR_H
#define TASKPROCESSOR_H

#include "Task.h"
#include "HomeAssistantAPI.h"
#include <functional>
#if defined(BUILD_SERVER) || defined(BUILD_FULL)
#include "ModelRunner.h"
#endif

class TaskProcessor
{
public:
    #if defined(BUILD_SERVER) || defined(BUILD_FULL)
    TaskProcessor(HomeAssistantAPI *homeAssistantAPI, ModelRunner &nerModel, ModelRunner &classificationModel);
    #else
    TaskProcessor(HomeAssistantAPI *homeAssistantAPI);
    #endif
    void processTask(const Task &task);
private:
    HomeAssistantAPI *homeAssistantAPI_;
    std::function<void(const Task &task)> taskHandler_;
    void processGeneralTask(const Task &task);
    void processHomeAssistantTask(const Task &task);
    #if defined(BUILD_SERVER) || defined(BUILD_FULL)
    ModelRunner &nerModel_;
    ModelRunner &classificationModel_;
    #endif
};

#endif