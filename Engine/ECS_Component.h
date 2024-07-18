#pragma once

#include "ECSDefine.h"

///////////////////////
// Component
///////////////////////
struct IComponentRegistry
{
	virtual ~IComponentRegistry() {}

	virtual uint32 GetComponentSize() abstract;
	virtual void AddComponent( Entity entity, const void* data ) abstract;
	virtual bool HasComponent( Entity entity ) abstract;
};

template <typename T>
struct ComponentRegistry : public IComponentRegistry
{
	ComponentRegistry()
	{
		memset( _registry.data(), 0, _registry.size() * sizeof( T ) );
	}

	virtual uint32 GetComponentSize() override { return sizeof( T ); }

	virtual void AddComponent( Entity entity, const void* data ) override
	{
		if( data )
		{
			memcpy( &_registry[ entity ], data, sizeof( T ) );
		}

		_signature[ entity ] = true;
	}

	virtual bool HasComponent( Entity entity ) override
	{
		return _signature[ entity ];
	}

	T& GetComponent( Entity entity )
	{
		return _registry[ entity ];
	}

private:
	std::array<T, NUM_ENTITY_MAX> _registry;
	std::bitset<NUM_ENTITY_MAX> _signature;
};

// TODO: Change name to Entity?
struct EntityKey
{
	std::bitset<ECSComponentType_Count> signature;
	uint32 blockIndex;
	uint32 blockElemIndex;
};

class ArcheType
{
public:
	// TODO: Merge input parameters?
	ArcheType( const std::initializer_list<ECSComponentType>& types, const std::initializer_list<uint32>& typeByteSizes )
		: _elemSize( 0 )
		, _elemCapacity( 0 )
		, _blockElemByteSize( 0 )
		, _typeByteSizes( typeByteSizes )
		, _typeIndices( types )
	{
		for( const ECSComponentType type : types )
		{
			_signature.set( type );
		}

		_typeByteOffsets.reserve( typeByteSizes.size() );
		_typeByteSizes.reserve( typeByteSizes.size() );
		for( const uint32 typeByteSize : typeByteSizes )
		{
			_typeByteOffsets.emplace_back( GetBlockElemSize() * _blockElemByteSize );
			_typeByteSizes.emplace_back( typeByteSize );
			_blockElemByteSize += typeByteSize;
		}
	}

	bool HasFreeElements() const { return _elemSize < _elemCapacity; }

	const uint32 GetBlockElemSize() const { return 128; }
	const uint32 GetBlockElemByteSize() const { return GetBlockElemSize() * _blockElemByteSize; }

	void AllocateBlock()
	{
		_blocks.emplace_back( static_cast< uint8* >( malloc( GetBlockElemByteSize() ) ) );

		_elemCapacity += GetBlockElemSize();
	}

	const EntityKey AddElement()
	{
		if( !HasFreeElements() )
		{
			AllocateBlock();
		}
		
		const uint32 elemIndex = _elemSize++;
		const uint32 blockIndex = _blocks.size() - 1;
		const uint32 blockElemIndex = elemIndex % GetBlockElemSize();
		return EntityKey{ _signature, blockIndex, blockElemIndex };
	}

	void RemoveElement( const uint32 elemIndexA )
	{
		const uint32 blockIndexA = elemIndexA / GetBlockElemSize();
		const uint32 blockElemIndexA = elemIndexA % GetBlockElemSize();

		const uint32 elemIndexB = _elemSize - 1;
		const uint32 blockElemIndexB = elemIndexB % GetBlockElemSize();

		for( uint32 typeIndex = 0; typeIndex < _typeByteSizes.size(); ++typeIndex )
		{
			const uint32 typeByteOffset = _typeByteOffsets[ typeIndex ];
			const uint32 typeByteSize = _typeByteSizes[ typeIndex ];

			uint8* memA = _blocks[ blockIndexA ] + typeByteOffset + blockElemIndexA * typeByteSize;
			uint8* memB = _blocks.back() + typeByteOffset + blockElemIndexB * typeByteSize;

			memcpy( memA, memB, typeByteSize );
		}

		_elemSize--;
	}

	// TODO: Need return value?
	bool GetTypeIndex( uint32& typeIndex, const ECSComponentType type )
	{
		for( uint32 index = 0; index < _typeIndices.size(); ++index )
		{
			if( _typeIndices[ index ] == type )
			{
				typeIndex = index;

				return true;
			}
		}

		return false;
	}

	bool GetTypeOffset( uint32& typeOffset, const ECSComponentType type )
	{
		uint32 typeIndex;
		if( GetTypeIndex( typeIndex, type ) )
		{
			typeOffset = _typeByteOffsets[ typeIndex ];

			return true;
		}
		
		return false;
	}

	// TODO: Not efficient. Use foreach loop instead.
	template<typename T = IECSComponent>
	T& GetElement( const ECSComponentType type, const uint32 elemIndex )
	{
		// TODO: Too slow..? Search element by key(block index + element index)?
		const uint32 blockIndex = elemIndex / GetBlockElemSize();
		const uint32 blockElemIndex = elemIndex % GetBlockElemSize();

		uint32 typeIndex;
		GetTypeIndex( typeIndex, type );
		const uint32 typeByteOffset = _typeByteOffsets[ typeIndex ];

		T* block = static_cast< T* >( _blocks[ blockIndex ] + typeByteOffset );

		return block[ blockElemIndex ];
	}

	template<typename T = IECSComponent>
	void ForEachElement( const ECSComponentType type, std::function<void(T&)> func )
	{
		uint32 typeIndex;
		GetTypeIndex( typeIndex, type );
		const uint32 typeByteOffset = _typeByteOffsets[ typeIndex ];

		for( uint32 blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex )
		{
			T* block = static_cast< T* >( _blocks[ blockIndex ] + typeByteOffset );

			for( uint32 blockElemIndex = 0; blockElemIndex < GetBlockElemSize(); ++blockElemIndex )
			{
				T& elem = block[ blockElemIndex ];
				func( elem );
			}
		}
	}

private:
	std::bitset<ECSComponentType_Count> _signature;

	uint32 _elemSize;
	uint32 _elemCapacity;
	uint32 _blockElemByteSize;

	std::vector<ECSComponentType> _typeIndices;
	std::vector<uint32> _typeByteOffsets;
	std::vector<uint32> _typeByteSizes;

	std::vector<uint8*> _blocks;
};
using ArcheTypeRef = std::shared_ptr<ArcheType>;

// TODO: Adjust size of archetype block by expected number of archetype elements

class ArcheTypeRegistry
{
public:
	bool ContainsArcheType( std::bitset<ECSComponentType_Count>& signature )
	{
		return _elements.contains( signature );
	}

	void InitArcheType( const std::initializer_list<ECSComponentType>& types, const std::initializer_list<uint32>& typeByteSizes )
	{
		std::bitset<ECSComponentType_Count> signature;

		for( const ECSComponentType type : types )
		{
			signature.set( type );
		}

		if( !ContainsArcheType( signature ) )
		{
			_elements[ signature ] = ArcheTypeRef( new ArcheType( types, typeByteSizes ) );
		}
	}

	ArcheTypeRef GetArcheType( std::bitset<ECSComponentType_Count>& signature )
	{
		return _elements[ signature ];
	}

	//std::vector<ArcheTypeRef>& GetArcheTypes( ECSComponentType type ) { return _elemMap[ type ]; }

private:
	std::unordered_map<std::bitset<ECSComponentType_Count>, ArcheTypeRef> _elements;

	//std::array<std::vector<ArcheTypeRef>, ECSComponentType_Count> _elemMap;
};
