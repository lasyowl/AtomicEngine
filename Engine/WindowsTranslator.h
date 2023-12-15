#pragma once

#include "Shape.h"
#include <Windows.h>

namespace Convert
{

SRect ToEngineRect( const RECT& InRect );
RECT ToWindowsRect( const SRect& InRect );

}