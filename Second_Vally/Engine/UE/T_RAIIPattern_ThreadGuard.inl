#pragma once

#include <utility>

template <typename ThreadGeneratedFunc, typename ThreadShutdownFunc>
TRAIIPattern_ThreadGuard<ThreadGeneratedFunc, ThreadShutdownFunc>::TRAIIPattern_ThreadGuard(
	ThreadGeneratedFunc _Initialize,
	ThreadShutdownFunc _Shutdown)
	: __Shutdown(std::move(_Shutdown))
{
	__Result = _Initialize();
	__bActive = SUCCEEDED(__Result);
}

template <typename ThreadGeneratedFunc, typename ThreadShutdownFunc>
TRAIIPattern_ThreadGuard<ThreadGeneratedFunc, ThreadShutdownFunc>::~TRAIIPattern_ThreadGuard()
{
	if (__bActive)
		__Shutdown();
}

template <typename ThreadGeneratedFunc, typename ThreadShutdownFunc>
bool TRAIIPattern_ThreadGuard<ThreadGeneratedFunc, ThreadShutdownFunc>::Failed() const
{
	return FAILED(__Result);
}
