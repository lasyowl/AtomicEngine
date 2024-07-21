#include <Engine/ArcheType.h>

///////////////////////
// ArcheType
///////////////////////
ArcheType::ArcheType( const ECSComponentType type, const uint32 typeByteSize )
	: _elemSize( 0 )
	, _elemCapacity( 0 )
{
	_signature.set( type );

	_blockElemByteSize = GetBlockElemSize() * typeByteSize;

	_typeInfos.emplace_back( type, 0, typeByteSize );
}

ArcheType::ArcheType( const std::vector<ArcheTypeInfo>& typeInfos )
	: _elemSize( 0 )
	, _elemCapacity( 0 )
	, _typeInfos( typeInfos )
{
	for( const ArcheTypeInfo& typeInfo : _typeInfos )
	{
		_signature.set( typeInfo.type );
	}

	GenerateTypeInfoByteOffsets( _typeInfos );

	const ArcheTypeInfo& typeInfoLast = _typeInfos.back();
	_blockElemByteSize = typeInfoLast.byteOffset + GetBlockElemSize() * typeInfoLast.byteSize;
}

ArcheTypeRef ArcheType::GenerateDerivedShrink( const ECSComponentType type, const uint32 typeByteSize ) const
{
	std::vector<ArcheTypeInfo> typeInfos = _typeInfos;

	std::erase_if( typeInfos, [ type ]( const ArcheTypeInfo& info )
				   {
					   return info.type == type;
				   } );

	return std::make_shared<ArcheType>( typeInfos );
}

ArcheTypeRef ArcheType::GenerateDerivedExpand( const ECSComponentType type, const uint32 typeByteSize ) const
{
	std::vector<ArcheTypeInfo> typeInfos = _typeInfos;

	// TODO : Sort by type?
	typeInfos.emplace_back( type, 0, typeByteSize );

	return std::make_shared<ArcheType>( typeInfos );
}

ArcheType::ElementKey ArcheType::AddElement()
{
	if( !HasFreeElements() )
	{
		AllocateBlock();
	}

	const uint32 elemIndex = _elemSize++;
	const uint32 blockIndex = _blocks.size() - 1;
	const uint32 blockElemIndex = elemIndex % GetBlockElemSize();
	return ElementKey{ _signature, blockIndex, blockElemIndex };
}

void ArcheType::RemoveElement( const ElementKey& key )
{
	const uint32 blockIndexA = key.blockIndex;
	const uint32 blockElemIndexA = key.blockElemIndex;

	const uint32 elemIndexB = _elemSize - 1;
	const uint32 blockElemIndexB = elemIndexB % GetBlockElemSize();

	for( uint32 typeIndex = 0; typeIndex < _typeInfos.size(); ++typeIndex )
	{
		const ArcheTypeInfo& typeInfo = _typeInfos[ typeIndex ];

		uint8* memA = _blocks[ blockIndexA ] + typeInfo.byteOffset + blockElemIndexA * typeInfo.byteSize;
		const uint8* memB = _blocks.back() + typeInfo.byteOffset + blockElemIndexB * typeInfo.byteSize;

		::memcpy( memA, memB, typeInfo.byteSize );
	}

	_elemSize--;
}

void ArcheType::CopyElementShrink( const ArcheType& A, const ArcheType& B, const ElementKey& keyA, const ElementKey& keyB )
{
	assert( A.GetTypeCount() < B.GetTypeCount() );

	const uint32 elemCount = B.GetTypeCount();

	uint32 indexA = 0;
	uint32 indexB = 0;
	while( indexB < elemCount )
	{
		const ArcheTypeInfo& infoA = A.GetTypeInfo( indexA );
		const ArcheTypeInfo& infoB = B.GetTypeInfo( indexB );
		if( infoA.type != infoB.type )
		{
			indexB++;
			continue;
		}

		uint8* blockPtrA = A._blocks[ keyA.blockIndex ] + infoA.byteOffset;
		uint8* blockPtrB = B._blocks[ keyB.blockIndex ] + infoB.byteOffset;

		uint8* elemPtrA = blockPtrA + infoA.byteSize * keyA.blockElemIndex;
		uint8* elemPtrB = blockPtrB + infoB.byteSize * keyB.blockElemIndex;

		::memcpy( elemPtrA, elemPtrB, infoA.byteSize );

		indexA++;
		indexB++;
	}
}

void ArcheType::CopyElementExpand( const ArcheType& A, const ArcheType& B, const ElementKey& keyA, const ElementKey& keyB )
{
	assert( A.GetTypeCount() > B.GetTypeCount() );

	const uint32 elemCount = B.GetTypeCount();

	for( uint32 index = 0; index < elemCount; ++index )
	{
		const ArcheTypeInfo& infoA = A.GetTypeInfo( index );
		const ArcheTypeInfo& infoB = B.GetTypeInfo( index );

		uint8* blockPtrA = A._blocks[ keyA.blockIndex ] + infoA.byteOffset;
		uint8* blockPtrB = B._blocks[ keyB.blockIndex ] + infoB.byteOffset;

		uint8* elemPtrA = blockPtrA + infoA.byteSize * keyA.blockElemIndex;
		uint8* elemPtrB = blockPtrB + infoB.byteSize * keyB.blockElemIndex;

		::memcpy( elemPtrA, elemPtrB, infoA.byteSize );
	}
}

const ArcheType::ArcheTypeInfo& ArcheType::GetTypeInfo( const ECSComponentType type ) const
{
	for( const ArcheTypeInfo& info : _typeInfos )
	{
		if( info.type == type )
		{
			return info;
		}
	}

	assert( false );

	return ArcheTypeInfo{};
}

const ArcheType::ArcheTypeInfo& ArcheType::GetTypeInfo( const uint32 index ) const
{
	return _typeInfos[ index ];
}

void ArcheType::AllocateBlock()
{
	_blocks.emplace_back( static_cast< uint8* >( malloc( GetBlockElemByteSize() ) ) );

	_elemCapacity += GetBlockElemSize();
}

void ArcheType::GenerateTypeInfoByteOffsets( std::vector<ArcheTypeInfo>& infos )
{
	infos[ 0 ].byteOffset = 0;

	for( uint32 index = 1; index < infos.size(); ++index )
	{
		infos[ index ].byteOffset = infos[ index - 1 ].byteOffset + GetBlockElemSize() * infos[ index - 1 ].byteSize;
	}
}
