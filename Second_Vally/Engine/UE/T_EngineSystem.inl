#include "Engine.h"

template<typename T>
TEngineSystem<T>::TEngineSystem()
{
    UEngine::GetInstance()->RegisterEngineSystem(this);
}