#include "PrimitiveTopology.h"

#include <d3dcommon.h>

namespace Convert
{
constexpr int32 PrimitiveTopologyToDirectX( EPrimitiveTopology Topology )
{
	switch ( Topology )
	{
		case EPrimitiveTopology::PointList:		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case EPrimitiveTopology::LineList:		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case EPrimitiveTopology::TriangleList:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
	return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}
}