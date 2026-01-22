#pragma once

#include <Core/IntVector.h>
#include <Core/Singleton.h>

enum EKeyboardInput : uint8;
enum EKeyboardInputState : uint8;
enum EMouseInput : uint8;
enum EMouseInputState : uint8;

using InputDelegate = std::function<void()>;

class IInputInterface
{
public:
    virtual ~IInputInterface() = default;

    // @TODO: Make abstract these functions
    virtual void RegisterOnKeyDown( EKeyboardInput key, InputDelegate delegate ) {}
    virtual void RegisterOnKeyPressed( EKeyboardInput key, InputDelegate delegate ) {}
    virtual void RegisterOnKeyUp( EKeyboardInput key, InputDelegate delegate ) {}

    virtual void RegisterOnMouseDown( EMouseInput key, InputDelegate delegate ) {}
    virtual void RegisterOnMousePressed( EMouseInput key, InputDelegate delegate ) {}
    virtual void RegisterOnMouseUp( EMouseInput key, InputDelegate delegate ) {}

    virtual const IVec2& GetMousePosition() const { return IVec2(); }

    virtual void Tick() abstract;

    virtual void ProcessInput( const MSG& message, HWND hWnd ) abstract;
};

class InputSystem : public TSingleton<InputSystem>
{
public:
    InputSystem();

    void BindKeyboardInputDelegate( EKeyboardInput key, EKeyboardInputState state, InputDelegate delegate );
    void BindMouseInputDelegate( EMouseInput mouse, EMouseInputState state, InputDelegate delegate );

    IVec2 GetMousePosition();

    void Tick();

    void ProcessInput( const MSG& message, HWND hWnd );

private:
    std::shared_ptr<IInputInterface> _keyboardInterface;
    std::shared_ptr<IInputInterface> _mouseInterface;
};
