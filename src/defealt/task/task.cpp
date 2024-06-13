#include "Task.h"

Task::Task(const std::string &desc, int prio, TaskType typ)
    : description(desc), priority(prio), type(typ) {}

Task::Task(const std::string &desc, const std::string &entityId, const std::string &service, const std::string &newState, int prio)
    : description(desc), entityId(entityId), service(service), newState(newState), priority(prio), type(HOME_ASSISTANT) {}
