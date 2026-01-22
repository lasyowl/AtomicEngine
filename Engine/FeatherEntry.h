#pragma once

namespace feather
{
    struct FLaunchParam
    {
        void* platformHandle;
    };

    void Launch( FLaunchParam& param );
};
