#pragma once

#include <Core/Singleton.h>

template<typename T>
class TCache : public TSingleton<TCache<T>>
{
public:
    std::shared_ptr<T> Add( const std::string& name, std::shared_ptr<T> item )
    {
        assert( !_cache.contains( name ) );
        _cache[ name ] = item;

        return item;
    }

    template<typename... Args>
    std::shared_ptr<T> FindOrAdd( const std::string& name, Args&&... args )
    {
        if ( _cache.contains( name ) )
            return _cache[ name ];

        _cache[ name ] = std::make_shared<T>( std::forward<Args>( args )... );
        return _cache[ name ];
    }

    std::shared_ptr<T> Find( const std::string& name )
    {
        return _cache.contains( name ) ? _cache[ name ] : nullptr;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<T>> _cache;
};
