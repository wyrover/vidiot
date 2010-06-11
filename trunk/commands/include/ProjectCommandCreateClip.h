#ifndef PROJECT_COMMAND_CREATE_CLIP_H
#define PROJECT_COMMAND_CREATE_CLIP_H

#include "ProjectCommand.h"

class ProjectCommandCreateClip : public ProjectCommand
{
public:
    ProjectCommandCreateClip();
    ~ProjectCommandCreateClip();
    bool Do();
    bool Undo();
private:
};

#endif // PROJECT_COMMAND_CREATE_CLIP_H
