#pragma once

#include <Engine/ECSDefine.h>

///////////////////////
// ArcheType
///////////////////////
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
	ArcheType( const ECSComponentType type, const uint32 typeByteSize );
	ArcheType( const std::vector<ArcheTypeInfo>& typeInfos );

	ArcheTypeRef GenerateDerivedShrink( const ECSComponentType type, const uint32 typeByteSize ) const;
	ArcheTypeRef GenerateDerivedExpand( const ECSComponentType type, const uint32 typeByteSize ) const;

	ElementKey AddElement();

	void RemoveElement( const ElementKey& key );

	/*
	* Copy element from ArcheType B to ArcheType A.
	* ArcheType A holds -1 component than ArcheType B.
	*/
	static void CopyElementShrink( const ArcheType& A, const ArcheType& B, const ElementKey& keyA, const ElementKey& keyB );

	/*
	* Copy element from ArcheType B to ArcheType A.
	* ArcheType A holds +1 component than ArcheType B.
	*/
	static void CopyElementExpand( const ArcheType& A, const ArcheType& B, const ElementKey& keyA, const ElementKey& keyB );

	template<typename T = IECSComponent>
	T& GetComponent( const ElementKey& key )
	{
		const ArcheTypeInfo& info = GetTypeInfo( T::type );

		T* block = reinterpret_cast< T* >( _blocks[ key.blockIndex ] + info.byteOffset );

		return block[ key.blockElemIndex ];
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

	const ArcheTypeInfo& GetTypeInfo( const ECSComponentType type ) const;
	const ArcheTypeInfo& GetTypeInfo( const uint32 index ) const;

	void AllocateBlock();

	void GenerateTypeInfoByteOffsets( std::vector<ArcheTypeInfo>& infos );

private:
	std::bitset<ECSComponentType_Count> _signature;

	uint32 _elemSize;
	uint32 _elemCapacity;
	uint32 _blockElemByteSize;

	std::vector<ArcheTypeInfo> _typeInfos;

	std::vector<uint8*> _blocks;
};

// TODO : Adjust size of archetype block by expected number of archetype elements

///////////////////////
// ArcheTypeRegistry
///////////////////////
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
