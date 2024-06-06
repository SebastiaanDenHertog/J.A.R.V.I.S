#ifndef TASK_H
#define TASK_H

#include <string>

class Task
{
public:
    std::string description;
    int priority;
    std::string type;

    Task(std::string desc, int prio, std::string typ);
};

#endif // TASK_H
