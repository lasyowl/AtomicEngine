#include "stdafx.h"
#include "WindowsTranslator.h"

namespace Convert
{

SRect ToEngineRect( const RECT& InRect )
{
	return SRect{ InRect.left, InRect.right, InRect.top, InRect.bottom };
}

RECT ToWindowsRect( const SRect& InRect )
{
	return RECT{ InRect.Left, InRect.Top, InRect.Right, InRect.Bottom };
}

}