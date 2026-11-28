#pragma once

#include <Windows.h>

template <typename ThreadGeneratedFunc, typename ThreadShutdownFunc>
class TRAIIPattern_ThreadGuard
{
public:
	TRAIIPattern_ThreadGuard(
		ThreadGeneratedFunc _Initialize,
		ThreadShutdownFunc _Shutdown);

	~TRAIIPattern_ThreadGuard();

	TRAIIPattern_ThreadGuard(const TRAIIPattern_ThreadGuard&) = delete;
	TRAIIPattern_ThreadGuard& operator=(const TRAIIPattern_ThreadGuard&) = delete;
	TRAIIPattern_ThreadGuard(TRAIIPattern_ThreadGuard&&) = delete;
	TRAIIPattern_ThreadGuard& operator=(TRAIIPattern_ThreadGuard&&) = delete;

	bool Failed() const;

private:
	ThreadShutdownFunc __Shutdown;
	HRESULT __Result = E_FAIL;
	bool __bActive = false;
};

#include "T_RAIIPattern_ThreadGuard.inl"
