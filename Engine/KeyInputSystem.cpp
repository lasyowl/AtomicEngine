#include "KeyInputSystem.h"

#include <Windows.h>

namespace
{
constexpr int16 KEY_PRESSED = 0x8000;

constexpr char keys[ KeyType_Count ] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
}

EKeyType ToEngineKeyType( char systemKey )
{
	switch( systemKey )
	{
		case 'A': return KeyType_A;	case 'B': return KeyType_B;	case 'C': return KeyType_C;	case 'D': return KeyType_D;
		case 'E': return KeyType_E;	case 'F': return KeyType_F;	case 'G': return KeyType_G;	case 'H': return KeyType_H;
		case 'I': return KeyType_I;	case 'J': return KeyType_J;	case 'K': return KeyType_K;	case 'L': return KeyType_L;
		case 'M': return KeyType_M;	case 'N': return KeyType_N;	case 'O': return KeyType_O;	case 'P': return KeyType_P;
		case 'Q': return KeyType_Q;	case 'R': return KeyType_R;	case 'S': return KeyType_S;	case 'T': return KeyType_T;
		case 'U': return KeyType_U;	case 'V': return KeyType_V;	case 'W': return KeyType_W;	case 'X': return KeyType_X;
		case 'Y': return KeyType_Y;	case 'Z': return KeyType_Z;
	}
	return KeyType_Unknown;
}

void KeyInputSystem::RunSystem( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<KeyInputComponent>* keyInputCompReg = GetRegistry<KeyInputComponent>( componentRegistry );
	if( !keyInputCompReg )
	{
		return;
	}

	KeyInputComponent& keyInputComp = keyInputCompReg->GetComponent( 0 );
	
	for( uint32 index = 0; index < KeyType_Count; ++index )
	{
		char key = keys[ index ];
		bool bPressed = GetKeyState( key ) & KEY_PRESSED;
		EKeyType keyType = ToEngineKeyType( key );
		bool bToggled = keyInputComp.keyPressed[ index ] != bPressed;

		keyInputComp.keyDown[ index ] = bPressed && bToggled;
		keyInputComp.keyUp[ index ] = !bPressed && bToggled;
		keyInputComp.keyPressed[ index ] = bPressed;

		/*if( keyInputComp.keyDown[ index ] )
		{
			wchar_t buffer[ 32 ];
			swprintf_s( buffer, L"%c down\n", key );
			OutputDebugStringW( buffer );
		}
		if( keyInputComp.keyUp[ index ] )
		{
			wchar_t buffer[ 32 ];
			swprintf_s( buffer, L"%c up\n", key );
			OutputDebugStringW( buffer );
		}
		if( keyInputComp.keyPressed[ index ] )
		{
			wchar_t buffer[ 32 ];
			swprintf_s( buffer, L"%c pressed\n", key );
			OutputDebugStringW( buffer );
		}*/
	}
}
