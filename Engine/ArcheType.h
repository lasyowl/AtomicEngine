#pragma once

#include <Engine/ECSDefine.h>

using ArcheTypeRef = std::shared_ptr<class ArcheType>;

class ArcheType
{
public:
	struct ElementKey
	{
		std::bitset<ECSComponentType_Count> signature;
		uint32 blockIndex;
		uint32 blockElemIndex;
	};

private:
	struct ArcheTypeInfo
	{
		ECSComponentType type;
		uint32 byteOffset;
		uint32 byteSize;
	};

public:
	ArcheType( const ECSComponentType type, const uint32 typeByteSize )
		: _elemSize( 0 )
		, _elemCapacity( 0 )
	{
		_signature.set( type );

		_blockElemByteSize = GetBlockElemSize() * typeByteSize;

		_typeInfos.emplace_back( type, 0, typeByteSize );
	}

	ArcheType( const std::vector<ArcheTypeInfo>& typeInfos )
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

	ArcheTypeRef GenerateDerivedShrink( const ECSComponentType type, const uint32 typeByteSize ) const
	{
		std::vector<ArcheTypeInfo> typeInfos = _typeInfos;

		std::erase_if( typeInfos, [ type ]( const ArcheTypeInfo& info )
					   {
						   return info.type == type;
					   } );

		return std::make_shared<ArcheType>( typeInfos );
	}

	ArcheTypeRef GenerateDerivedExpand( const ECSComponentType type, const uint32 typeByteSize ) const
	{
		std::vector<ArcheTypeInfo> typeInfos = _typeInfos;

		// TODO : Sort by type?
		typeInfos.emplace_back( type, 0, typeByteSize );

		return std::make_shared<ArcheType>( typeInfos );
	}

	ElementKey AddElement()
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

	void RemoveElement( const ElementKey& key )
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

	const ArcheTypeInfo& GetTypeInfo( const ECSComponentType type ) const
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

	const ArcheTypeInfo& GetTypeInfo( const uint32 index ) const
	{
		return _typeInfos[ index ];
	}

	template<typename T = IECSComponent>
	T& GetComponent( const ElementKey& key )
	{
		const ArcheTypeInfo& info = GetTypeInfo( T::type );

		T* block = reinterpret_cast< T* >( _blocks[ key.blockIndex ] + info.byteOffset );

		return block[ key.blockElemIndex ];
	}

	/*
	* Copy element from ArcheType B to ArcheType A.
	* ArcheType A holds -1 component than ArcheType B.
	*/
	static void CopyElementShrink( const ArcheType& A, const ArcheType& B, const ElementKey& keyA, const ElementKey& keyB )
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

	/*
	* Copy element from ArcheType B to ArcheType A.
	* ArcheType A holds +1 component than ArcheType B.
	*/
	static void CopyElementExpand( const ArcheType& A, const ArcheType& B, const ElementKey& keyA, const ElementKey& keyB )
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

	/*template<typename T = IECSComponent>
	void ForEachComponent( std::function<void(T&)> func )
	{
		uint32 typeIndex;
		GetTypeIndex( typeIndex, T::type );
		const uint32 typeByteOffset = _typeByteOffsets[ typeIndex ];

		for( uint32 blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex )
		{
			T* block = reinterpret_cast< T* >( _blocks[ blockIndex ] + typeByteOffset );

			for( uint32 blockElemIndex = 0; blockElemIndex < GetBlockElemSize(); ++blockElemIndex )
			{
				T& elem = block[ blockElemIndex ];
				func( elem );
			}
		}
	}*/
private:
	bool HasFreeElements() const { return _elemSize < _elemCapacity; }

	const uint32 GetTypeCount() const { return _typeInfos.size(); }
	const uint32 GetBlockElemSize() const { return 128; }
	const uint32 GetBlockElemByteSize() const { return GetBlockElemSize() * _blockElemByteSize; }

	void AllocateBlock()
	{
		_blocks.emplace_back( static_cast< uint8* >( malloc( GetBlockElemByteSize() ) ) );

		_elemCapacity += GetBlockElemSize();
	}

	void GenerateTypeInfoByteOffsets( std::vector<ArcheTypeInfo>& infos )
	{
		infos[ 0 ].byteOffset = 0;

		for( uint32 index = 1; index < infos.size(); ++index )
		{
			infos[ index ].byteOffset = infos[ index - 1 ].byteOffset + GetBlockElemSize() * infos[ index - 1 ].byteSize;
		}
	}

private:
	std::bitset<ECSComponentType_Count> _signature;

	uint32 _elemSize;
	uint32 _elemCapacity;
	uint32 _blockElemByteSize;

	std::vector<ArcheTypeInfo> _typeInfos;

	std::vector<uint8*> _blocks;
};

// TODO : Adjust size of archetype block by expected number of archetype elements

class ArcheTypeRegistry
{
public:
	template<typename T = IECSComponent>
	T& GetComponent( const ArcheType::ElementKey& key )
	{
		assert( _elements.contains( key.signature ) );

		ArcheTypeRef& archeType = _elements[ key.signature ];

		return archeType->GetComponent<T>( key );
	}

	/* Add a component to ArcheType with one or more components. */
	template<typename T = IECSComponent>
	const ArcheType::ElementKey AddComponent( const ArcheType::ElementKey& inKey, const T& inComponent )
	{
		assert( _elements.contains( inKey.signature ) );

		std::bitset<ECSComponentType_Count> signature = inKey.signature;
		signature.set( T::type );

		ArcheTypeRef& archeTypeSrc = _elements[ inKey.signature ];
		ArcheTypeRef& archeTypeDst = _elements[ signature ];
		if( !archeTypeDst )
		{
			archeTypeDst = archeTypeSrc->GenerateDerivedExpand( T::type, sizeof( T ) );
		}

		const ArcheType::ElementKey outKey = archeTypeDst->AddElement();
		ArcheType::CopyElementExpand( *archeTypeDst, *archeTypeSrc, outKey, inKey );

		archeTypeSrc->RemoveElement( inKey );

		return outKey;
	}

	/* ArcheType with only one component. */
	template<typename T = IECSComponent>
	const ArcheType::ElementKey AddComponent( const T& inComponent )
	{
		std::bitset<ECSComponentType_Count> signature;
		signature.set( T::type );

		ArcheTypeRef& archeType = _elements[ signature ];
		if( !archeType )
		{
			archeType = std::make_shared<ArcheType>( T::type, sizeof( T ) );
		}

		const ArcheType::ElementKey key = archeType->AddElement();

		T& component = archeType->GetComponent<T>( key );
		component = inComponent;

		return key;
	}

	template<typename T = IECSComponent>
	const ArcheType::ElementKey RemoveComponent( const ArcheType::ElementKey& inKey )
	{
		assert( _elements.contains( inKey.signature ) );

		ArcheTypeRef& archeTypeSrc = _elements[ inKey.signature ];

		std::bitset<ECSComponentType_Count> signature = inKey.signature;
		signature.set( T::type, false );

		ArcheType::ElementKey outKey = { 0, 0, 0 };

		if( !signature.none() )
		{
			ArcheTypeRef& archeTypeDst = _elements[ signature ];
			if( !archeTypeDst )
			{
				archeTypeDst = archeTypeSrc->GenerateDerivedShrink( T::type, sizeof( T ) );
			}

			outKey = archeTypeDst->AddElement();

			ArcheType::CopyElementShrink( *archeTypeDst, *archeTypeSrc, outKey, inKey );
		}

		archeTypeSrc->RemoveElement( inKey );

		return outKey;
	}

private:
	//std::vector<ArcheTypeRef>& GetArcheTypes( ECSComponentType type ) { return _elemMap[ type ]; }

private:
	std::unordered_map<std::bitset<ECSComponentType_Count>, ArcheTypeRef> _elements;

	//std::array<std::vector<ArcheTypeRef>, ECSComponentType_Count> _elemMap;
};
