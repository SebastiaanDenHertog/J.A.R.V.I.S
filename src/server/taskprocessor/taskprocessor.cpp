#include "TaskProcessor.h"
#include "MediaPlayer.h"
#include <iostream>


#if defined(BUILD_SERVER) || defined(BUILD_FULL)
    TaskProcessor::TaskProcessor(HomeAssistantAPI *homeAssistantAPI, ModelRunner &nerModel, ModelRunner &classificationModel):
    homeAssistantAPI_(homeAssistantAPI), nerModel_(nerModel), classificationModel_(classificationModel)
    {
        // Initialize taskHandler_ with a valid function
        taskHandler_ = [this](const Task &task)
        {
            // Example task handling code
            std::cout << "Handling task: " << task.description << std::endl;
            // Additional task processing logic
        };
    }
#else
    TaskProcessor::TaskProcessor(HomeAssistantAPI *homeAssistantAPI)
        : homeAssistantAPI_(homeAssistantAPI)
    {
        // Initialize taskHandler_ with a valid function
        taskHandler_ = [this](const Task &task)
        {
            // Example task handling code
            std::cout << "Handling task: " << task.description << std::endl;
            // Additional task processing logic
        };
    }
#endif


void TaskProcessor::processTask(const Task &task)
{
    // Ignore empty tasks
    if (task.description.empty())
    {
        std::cout << "Received an empty task, ignoring." << std::endl;
        return;
    }

    switch (task.type)
    {
    case Task::Book:
        // Add your code to handle booking here
        break;
    case Task::Calculate:
        // Add your code to handle calculation here
        break;
    case Task::Calendar:
        // Add your code to handle calendar here
        break;
    case Task::Call:
        // Add your code to handle calling here
        break;
    case Task::Connect:
        // Add your code to handle connecting here
        break;
    case Task::ControlHeating:
        processHomeAssistantTask(task);
        // Add your code to control heating here
        break;
    case Task::ControlLight:
        processHomeAssistantTask(task);
        // Add your code to control light here
        break;
    case Task::Define:
        // Add your code to define here
        break;
    case Task::Email:
        // Add your code to handle emails here
        break;
    case Task::Find:
        // Add your code to find here
        break;
    case Task::GetRecipe:
        // Add your code to get a recipe here
        break;
    case Task::GetShippingInfo:
        // Add your code to get shipping info here
        break;
    case Task::Locate:
        // Add your code to locate here
        break;
    case Task::Message:
        // Add your code to send a message here
        break;
    case Task::Navigate:
        // Add your code to navigate here
        break;
    case Task::NewsQuery:
        // Add your code to query the news here
        break;
    case Task::OrderItem:
        // Add your code to order an item here
        break;
    case Task::PauseMusic:
        // Add your code to pause music here
        break;
    case Task::PauseVideo:
        // Add your code to pause video here
        break;
    case Task::PlayMusic:
    {
        MediaPlayer player;
        player.setoutput(task.device, task.output);
        player.play(player.FindSong(task.entities));
        break;
    }
    case Task::PlayVideo:
        // Add your code to play video here
        break;
    case Task::Read:
        // Add your code to read here
        break;
    case Task::Recommend:
        // Add your code to recommend here
        break;
    case Task::ResumeVideo:
        // Add your code to resume video here
        break;
    case Task::SetAlarm:
        // Add your code to set an alarm here
        break;
    case Task::SetTimer:
        // Add your code to set a timer here
        break;
    case Task::SetVolume:
        // Add your code to set the volume here
        break;
    case Task::ShoppingList:
        // Add your code to handle shopping list here
        break;
    case Task::Summarize:
        // Add your code to summarize here
        break;
    case Task::Translate:
        // Add your code to translate here
        break;
    case Task::WeatherQuery:
        // Add your code to query the weather here
        break;
    case Task::ERROR:
        std::cerr << "Error task received: " << task.description << std::endl;
        break;
    default:
        std::cerr << "Unknown task type received: " << task.description << std::endl;
        break;
    }
}

void TaskProcessor::processGeneralTask(const Task &task)
{
    // General task processing logic here
    std::cout << "Processing general task: " << task.description << std::endl;
}

void TaskProcessor::processHomeAssistantTask(const Task &task)
{
    std::cout << "Processing Home Assistant task: " << task.description << std::endl;
    if (homeAssistantAPI_)
    {
        if (!task.service.empty())
        {
            homeAssistantAPI_->callService("homeassistant", task.service, task.entityId);
        }
        else if (!task.newState.empty())
        {
            homeAssistantAPI_->sendStateChange(task.entityId, task.newState);
        }
    }
    else
    {
        std::cerr << "Home Assistant API is not initialized." << std::endl;
    }

}
