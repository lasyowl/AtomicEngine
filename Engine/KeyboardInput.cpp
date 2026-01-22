#include "KeyboardInput.h"

void KeyboardInputInterface::RegisterOnKeyDown( EKeyboardInput key, InputDelegate delegate )
{
    // Engine thread only
    _onKeyDown[ key ] = delegate;
}

void KeyboardInputInterface::RegisterOnKeyPressed( EKeyboardInput key, InputDelegate delegate )
{
    // Engine thread only
    _onKeyPressed[ key ] = delegate;
}

void KeyboardInputInterface::RegisterOnKeyUp( EKeyboardInput key, InputDelegate delegate )
{
    // Engine thread only
    _onKeyUp[ key ] = delegate;
}

void KeyboardInputInterface::Tick()
{
    for ( int32 index = 0; index < Key_Count; ++index )
    {
        if ( !_isKeyDown[ index ] )
            continue;

        const EKeyboardInput key = static_cast<EKeyboardInput>( index );

        if ( _onKeyPressed.contains( key ) )
            _onKeyPressed[ key ]();
    }
}

void KeyboardInputInterface::ProcessInput( const MSG& message, HWND hWnd )
{
    if ( message.message != WM_KEYDOWN && message.message != WM_KEYUP )
        return;

    EKeyboardInput key;

    switch ( message.wParam )
    {
        case 'a': key = Key_A; break;
        case 'b': key = Key_B; break;
        case 'c': key = Key_C; break;
        case 'd': key = Key_D; break;
        case 'e': key = Key_E; break;
        case 'f': key = Key_F; break;
        case 'g': key = Key_G; break;
        case 'h': key = Key_H; break;
        case 'i': key = Key_I; break;
        case 'j': key = Key_J; break;
        case 'k': key = Key_K; break;
        case 'l': key = Key_L; break;
        case 'm': key = Key_M; break;
        case 'n': key = Key_N; break;
        case 'o': key = Key_O; break;
        case 'p': key = Key_P; break;
        case 'q': key = Key_Q; break;
        case 'r': key = Key_R; break;
        case 's': key = Key_S; break;
        case 't': key = Key_T; break;
        case 'u': key = Key_U; break;
        case 'v': key = Key_V; break;
        case 'w': key = Key_W; break;
        case 'x': key = Key_X; break;
        case 'y': key = Key_Y; break;
        case 'z': key = Key_Z; break;
        case 'A': key = Key_A; break;
        case 'B': key = Key_B; break;
        case 'C': key = Key_C; break;
        case 'D': key = Key_D; break;
        case 'E': key = Key_E; break;
        case 'F': key = Key_F; break;
        case 'G': key = Key_G; break;
        case 'H': key = Key_H; break;
        case 'I': key = Key_I; break;
        case 'J': key = Key_J; break;
        case 'K': key = Key_K; break;
        case 'L': key = Key_L; break;
        case 'M': key = Key_M; break;
        case 'N': key = Key_N; break;
        case 'O': key = Key_O; break;
        case 'P': key = Key_P; break;
        case 'Q': key = Key_Q; break;
        case 'R': key = Key_R; break;
        case 'S': key = Key_S; break;
        case 'T': key = Key_T; break;
        case 'U': key = Key_U; break;
        case 'V': key = Key_V; break;
        case 'W': key = Key_W; break;
        case 'X': key = Key_X; break;
        case 'Y': key = Key_Y; break;
        case 'Z': key = Key_Z; break;
        case '0': key = Key_0; break;
        case '1': key = Key_1; break;
        case '2': key = Key_2; break;
        case '3': key = Key_3; break;
        case '4': key = Key_4; break;
        case '5': key = Key_5; break;
        case '6': key = Key_6; break;
        case '7': key = Key_7; break;
        case '8': key = Key_8; break;
        case '9': key = Key_9; break;
        case VK_ESCAPE: key = Key_Escape; break;
        case VK_SPACE: key = Key_Space; break;
        case VK_RETURN: key = Key_Enter; break;
        case VK_LEFT: key = Key_Left; break;
        case VK_RIGHT: key = Key_Right; break;
        case VK_UP: key = Key_Up; break;
        case VK_DOWN: key = Key_Down; break;
        default: key = Key_None; break;
    }

    if ( key == Key_None )
        return;

    const bool isKeyDown = message.message == WM_KEYDOWN;
    if ( _isKeyDown[ key ] != isKeyDown )
    {
        if ( isKeyDown )
        {
            if ( _onKeyDown.contains( key ) )
                _onKeyDown[ key ]();
        }
        else
        {
            if ( _onKeyUp.contains( key ) )
                _onKeyUp[ key ]();
        }

        _isKeyDown[ key ] = isKeyDown;
    }
}
