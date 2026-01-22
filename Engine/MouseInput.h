#pragma once

#include <Core/IntVector.h>
#include <Engine/InputSystem.h>

enum EMouseInput : uint8
{
    Mouse_Begin,
    Mouse_LeftButton = Mouse_Begin,
    Mouse_RightButton,
    Mouse_MiddleButton,
    Mouse_End = Mouse_MiddleButton + 1,
    Mouse_Count = Mouse_End - Mouse_Begin,
    Mouse_None
};

enum EMouseInputState : uint8
{
    MouseState_Down,
    MouseState_Pressed,
    MouseState_Up,
};

class MouseInputInterface : public IInputInterface
{
public:
    virtual void RegisterOnMouseDown( EMouseInput button, InputDelegate delegate ) override;
    virtual void RegisterOnMousePressed( EMouseInput button, InputDelegate delegate ) override;
    virtual void RegisterOnMouseUp( EMouseInput button, InputDelegate delegate ) override;

    virtual const IVec2& GetMousePosition() const { return _mousePosition; }

    virtual void Tick() override;

    virtual void ProcessInput( const MSG& message, HWND hWnd ) override;

private:
    std::unordered_map<EMouseInput, InputDelegate> _onButtonDown;
    std::unordered_map<EMouseInput, InputDelegate> _onButtonPressed;
    std::unordered_map<EMouseInput, InputDelegate> _onButtonUp;

    std::bitset<Mouse_Count> _isButtonDown;

    IVec2 _mousePosition;
};
