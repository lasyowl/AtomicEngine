#include "InputSystem.h"

#include <Engine/KeyboardInput.h>
#include <Engine/MouseInput.h>

InputSystem::InputSystem()
{
    _keyboardInterface = std::make_shared<KeyboardInputInterface>();
    _mouseInterface = std::make_shared<MouseInputInterface>();
}

void InputSystem::BindKeyboardInputDelegate( EKeyboardInput key, EKeyboardInputState state, InputDelegate delegate )
{
    if ( state == KeyState_Down )
    {
        _keyboardInterface->RegisterOnKeyDown( key, delegate );
    }
    else if ( state == KeyState_Pressed )
    {
        _keyboardInterface->RegisterOnKeyPressed( key, delegate );
    }
    else if ( state == KeyState_Up )
    {
        _keyboardInterface->RegisterOnKeyUp( key, delegate );
    }
}

void InputSystem::BindMouseInputDelegate( EMouseInput mouse, EMouseInputState state, InputDelegate delegate )
{
    if ( state == MouseState_Down )
    {
        _mouseInterface->RegisterOnMouseDown( mouse, delegate );
    }
    else if ( state == MouseState_Pressed )
    {
        _mouseInterface->RegisterOnMousePressed( mouse, delegate );
    }
    else if ( state == MouseState_Up )
    {
        _mouseInterface->RegisterOnMouseUp( mouse, delegate );
    }
}

IVec2 InputSystem::GetMousePosition()
{
    return _mouseInterface->GetMousePosition();
}

void InputSystem::Tick()
{
    _keyboardInterface->Tick();
    _mouseInterface->Tick();
}

void InputSystem::ProcessInput( const MSG& message, HWND hWnd )
{
    _keyboardInterface->ProcessInput( message, hWnd );
    _mouseInterface->ProcessInput( message, hWnd );
}
