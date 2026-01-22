#include "ViewController.h"

#include <Engine/KeyboardInput.h>
#include <Engine/MouseInput.h>

ViewController::ViewController( ViewObject& viewObject )
    : _viewObject( viewObject )
    , _isLocked( true )
{
    constexpr float moveSpeed = 5.0f;

    InputSystem::GetInstance().BindKeyboardInputDelegate( Key_W, KeyState_Pressed, [ this ]() { Move( _viewObject.GetFront() * moveSpeed ); } );
    InputSystem::GetInstance().BindKeyboardInputDelegate( Key_A, KeyState_Pressed, [ this ]() { Move( _viewObject.GetRight() * -moveSpeed ); } );
    InputSystem::GetInstance().BindKeyboardInputDelegate( Key_S, KeyState_Pressed, [ this ]() { Move( _viewObject.GetFront() * -moveSpeed ); } );
    InputSystem::GetInstance().BindKeyboardInputDelegate( Key_D, KeyState_Pressed, [ this ]() { Move( _viewObject.GetRight() * moveSpeed ); } );
    InputSystem::GetInstance().BindKeyboardInputDelegate( Key_Q, KeyState_Pressed, [ this ]() { Move( _viewObject.GetUp() * -moveSpeed ); } );
    InputSystem::GetInstance().BindKeyboardInputDelegate( Key_E, KeyState_Pressed, [ this ]() { Move( _viewObject.GetUp() * moveSpeed ); } );

    InputSystem::GetInstance().BindMouseInputDelegate( Mouse_RightButton, MouseState_Down, [ this ]()
    {
        _isLocked = false;
        _prevMousePosition = InputSystem::GetInstance().GetMousePosition();
    } );
    InputSystem::GetInstance().BindMouseInputDelegate( Mouse_RightButton, MouseState_Up, [ this ]()
    {
        _isLocked = true;
    } );
}

void ViewController::Tick()
{
    if ( !_isLocked )
        Rotate();
}

void ViewController::Move( const Vec3& delta )
{
    if ( !_isLocked )
    {
        _viewObject.Move( delta );
    }
}

void ViewController::Rotate()
{
    const IVec2 mousePosition = InputSystem::GetInstance().GetMousePosition();
    const IVec2 delta = mousePosition - _prevMousePosition;
    if ( delta.x != 0 || delta.y != 0 )
    {
        _mousePositionDelta += delta;

        constexpr float rotateSpeed = 0.0025f;
        Quaternion rotation = Quaternion( DirectX::XMQuaternionRotationRollPitchYaw( _mousePositionDelta.y * rotateSpeed, _mousePositionDelta.x * rotateSpeed, 0 ) );
        _viewObject.SetRotation( rotation );
    }

    _prevMousePosition = mousePosition;
}
