#include "MouseInput.h"

#include <windowsx.h>

void MouseInputInterface::RegisterOnMouseDown( EMouseInput button, InputDelegate delegate )
{
    // Engine thread only
    _onButtonDown[ button ] = delegate;
}

void MouseInputInterface::RegisterOnMousePressed( EMouseInput button, InputDelegate delegate )
{
    // Engine thread only
    _onButtonPressed[ button ] = delegate;
}

void MouseInputInterface::RegisterOnMouseUp( EMouseInput button, InputDelegate delegate )
{
    // Engine thread only
    _onButtonUp[ button ] = delegate;
}

void MouseInputInterface::Tick()
{
    for ( int32 index = 0; index < Mouse_Count; ++index )
    {
        if ( !_isButtonDown[ index ] )
            continue;

        const EMouseInput button = static_cast<EMouseInput>( index );

        if ( _onButtonPressed.contains( button ) )
            _onButtonPressed[ button ]();
    }
}

IVec2 decode_mouse_position( LPARAM lParam )
{
    return IVec2( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
}

void MouseInputInterface::ProcessInput( const MSG& message, HWND hWnd )
{
    EMouseInput button = Mouse_None;
    switch ( message.message )
    {
        case WM_LBUTTONDOWN:
        {
            SetCapture( hWnd );
            _mousePosition = decode_mouse_position( message.lParam );
            button = Mouse_LeftButton;
            break;
        }
        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            _mousePosition = decode_mouse_position( message.lParam );
            button = Mouse_LeftButton;
            break;
        }
        case WM_RBUTTONDOWN:
        {
            SetCapture( hWnd );
            _mousePosition = decode_mouse_position( message.lParam );
            button = Mouse_RightButton;
            break;
        }
        case WM_RBUTTONUP:
        {
            ReleaseCapture();
            _mousePosition = decode_mouse_position( message.lParam );
            button = Mouse_RightButton;
            break;
        }
        case WM_MOUSEMOVE:
        {
            _mousePosition = decode_mouse_position( message.lParam );
            break;
        }
        // @TODO: case WM_CAPTURECHANGED:
    }

    if ( button == Mouse_None )
        return;
     
    const bool isButtonDown = message.message == WM_LBUTTONDOWN || message.message == WM_RBUTTONDOWN;
    if ( _isButtonDown[ button ] != isButtonDown )
    {
        if ( isButtonDown )
        {
            if ( _onButtonDown.contains( button ) )
                _onButtonDown[ button ]();
        }
        else
        {
            if ( _onButtonUp.contains( button ) )
                _onButtonUp[ button ]();
        }

        _isButtonDown[ button ] = isButtonDown;
    }
}
