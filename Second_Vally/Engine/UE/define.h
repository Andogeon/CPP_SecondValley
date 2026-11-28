#pragma once

#if defined(_MSC_VER)
    #if defined(UE_EXPORTS)
        #define UE_MODULE_API __declspec(dllexport)
    #else
        #define UE_MODULE_API __declspec(dllimport)
    #endif

    #if defined(PROJECT_EXPORTS)
        #define PROJECT_API __declspec(dllexport)
    #else
        #define PROJECT_API __declspec(dllimport)
    #endif
#else
    #define UE_MODULE_API
    #define PROJECT_API
#endif

#ifndef CORE_API
#define CORE_API UE_MODULE_API
#endif

#ifndef COREUOBJECT_API
#define COREUOBJECT_API UE_MODULE_API
#endif

#ifndef UE_WITH_COREUOBJECT
#define UE_WITH_COREUOBJECT 0
#endif

#ifndef ENGINE_API
#define ENGINE_API UE_MODULE_API
#endif

#ifndef LAUNCH_API
#define LAUNCH_API UE_MODULE_API
#endif

#ifndef RHI_API
#define RHI_API UE_MODULE_API
#endif


#ifndef _TCHAR_DEFINED
using TCHAR = wchar_t;
#define _TCHAR_DEFINED
#endif

#ifndef TEXT
#define UE_TEXT_LITERAL_IMPL(Value) L##Value
#define TEXT(Value) UE_TEXT_LITERAL_IMPL(Value)
#endif

























#define DECLARE_SINGLETON(_UserDefineDataType)      friend class TSingleton<_UserDefineDataType>;   \
                                                    private:                                        \
                                                        _UserDefineDataType() = default;

#define GEngine UEngine::GetInstance()

#define DT AppTimeEngineSystem::GetInstance()->GetDeltaTime()

#define KEY_CONFIRM(_key, _State) InputEngineSystem::GetInstance()->GetKeyState(_key) == _State

#define KEY_PRESSED(_Key)       KEY_CONFIRM(_Key, EKeyState::Pressed)
#define KEY_REPEAT(_Key)        KEY_CONFIRM(_Key, EKeyState::Repeat)
#define KEY_RELEASED(_Key)      KEY_CONFIRM(_Key, EKeyState::Released)
#define KEY_NONE(_Key)          KEY_CONFIRM(_Key, EKeyState::None)

/*
	#define KEY_PRESSED(_Key)   InputEngineSystem::GetInstance()->IsKeyPressed(_Key)
	#define KEY_REPEAT(_Key)    InputEngineSystem::GetInstance()->IsKeyRepeat(_Key)
	#define KEY_RELEASED(_Key)  InputEngineSystem::GetInstance()->IsKeyReleased(_Key)
*/

#ifdef _MSC_VER
    #define FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCEINLINE __attribute__((always_inline)) inline
#else
    #define FORCEINLINE inline
#endif

#define Super       __super

#define ABSTRACT    abstract

#ifndef check
#define check(Expression) assert(Expression)
#endif

#ifndef checkSlow
#define checkSlow(Expression) assert(Expression)
#endif

#ifndef checkf
#define checkf(Expression, Format, ...) assert(Expression)
#endif

#ifndef ensure
#define ensure(Expression) (!!(Expression))
#endif

#ifndef ensureMsgf
#define ensureMsgf(Expression, Format, ...) (!!(Expression))
#endif
