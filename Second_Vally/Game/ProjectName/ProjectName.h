#pragma once

#include "UE/define.h"

class APlayer;

PROJECT_API bool InitializeProject();
PROJECT_API void DeinitializeProject();
PROJECT_API APlayer* GetProjectPlayer();
