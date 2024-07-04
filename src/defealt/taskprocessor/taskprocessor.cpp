#include "TaskProcessor.h"
#include "MediaPlayer.h"
#include <iostream>

TaskProcessor::TaskProcessor(ModelRunner &runner, HomeAssistantAPI *homeAssistantAPI)
    : modelRunner_(runner), homeAssistantAPI_(homeAssistantAPI)
{
    // Initialize taskHandler_ with a valid function
    taskHandler_ = [this](const Task &task)
    {
        // Example task handling code
        std::cout << "Handling task: " << task.description << std::endl;
        // Additional task processing logic
    };
}

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
    case Task::ControlLight:
        // Add your code to control light here
        processHomeAssistantTask(task);
        break;
    case Task::SetTimer:
        // Add your code to set a timer here
        break;
    case Task::PlayMusic:
    {
        MediaPlayer player;
        // Add your code to handle other cases here
        player.setoutput(task.device, task.output);
        player.play(player.FindSong(task.entities));
    }
    break;
    case Task::SetReminder:
        // Add your code to set a reminder here
        break;
    case Task::WeatherQuery:
        // Add your code to query the weather here
        break;
    case Task::SetAlarm:
        // Add your code to set an alarm here
        break;
    case Task::SetTemperature:
        processHomeAssistantTask(task);
        // Add your code to set the temperature here
        break;
    case Task::GetNews:
        // Add your code to get news here
        break;
    case Task::Lock:
        // Add your code to lock here
        break;
    case Task::Open:
        // Add your code to open here
        break;
    case Task::Close:
        // Add your code to close here
        break;
    case Task::Start:
        // Add your code to start here
        break;
    case Task::Stop:
        // Add your code to stop here
        break;
    case Task::Pause:
        // Add your code to pause here
        break;
    case Task::Resume:
        // Add your code to resume here
        break;
    case Task::TurnOn:
        processHomeAssistantTask(task);
        // Add your code to turn on here
        break;
    case Task::TurnOff:
        processHomeAssistantTask(task);
        // Add your code to turn off here
        break;
    case Task::Check:
        // Add your code to check here
        break;
    case Task::Read:
        // Add your code to read here
        break;
    case Task::Find:
        // Add your code to find here
        break;
    case Task::Locate:
        // Add your code to locate here
        break;
    case Task::Book:
        // Add your code to book here
        break;
    case Task::Call:
        // Add your code to call here
        break;
    case Task::Message:
        // Add your code to message here
        break;
    case Task::Water:
        processHomeAssistantTask(task);
        // Add your code to water here
        break;
    case Task::Question:
        // Add your code to handle question here
        break;
    case Task::Command:
        // Add your code to handle command here
        break;
    case Task::Information:
        // Add your code to handle information here
        break;
    case Task::SetVolume:
        // Add your code to set volume here
        break;
    case Task::GetShippingInfo:
        // Add your code to get shipping info here
        break;
    case Task::ReadMessages:
        // Add your code to read messages here
        break;
    case Task::GetRecipe:
        // Add your code to get recipe here
        break;
    case Task::Connect:
        // Add your code to handle connect here
        break;
    case Task::PlayVideo:
        // Add your code to play video here
        break;
    case Task::ShoppingList:
        // Add your code to handle shopping list here
        break;
    case Task::Calendar:
        // Add your code to handle calendar here
        break;
    case Task::Info:
        // Add your code to handle info here
        break;
    case Task::GetTraffic:
        // Add your code to get traffic here
        break;
    case Task::OrderItem:
        // Add your code to order item here
        break;
    case Task::SetMachine:
        // Add your code to set machine here
        break;
    case Task::SentMessage:
        // Add your code to send message here
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
