#pragma once

#include <Engine/InputSystem.h>

enum EKeyboardInput : uint8
{
    Key_Begin,
    Key_A = Key_Begin,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,
    Key_0,
    Key_1,
    Key_2,
    Key_3,
    Key_4,
    Key_5,
    Key_6,
    Key_7,
    Key_8,
    Key_9,
    Key_Escape,
    Key_Space,
    Key_Enter,
    Key_Left,
    Key_Right,
    Key_Up,
    Key_Down,
    Key_End = Key_Down + 1,
    Key_Count = Key_End - Key_Begin,
    Key_None
};

enum EKeyboardInputState : uint8
{
    KeyState_Down,
    KeyState_Pressed,
    KeyState_Up,
};

class KeyboardInputInterface : public IInputInterface
{
public:
    virtual void RegisterOnKeyDown( EKeyboardInput key, InputDelegate delegate ) override;
    virtual void RegisterOnKeyPressed( EKeyboardInput key, InputDelegate delegate ) override;
    virtual void RegisterOnKeyUp( EKeyboardInput key, InputDelegate delegate ) override;

    virtual void Tick() override;

    virtual void ProcessInput( const MSG& message, HWND hWnd ) override;

private:
    std::unordered_map<EKeyboardInput, InputDelegate> _onKeyDown;
    std::unordered_map<EKeyboardInput, InputDelegate> _onKeyPressed;
    std::unordered_map<EKeyboardInput, InputDelegate> _onKeyUp;

    std::bitset<Key_Count> _isKeyDown;
};
