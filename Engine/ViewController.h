#pragma once

#include "EngineObject.h"

class ViewController : public IEngineObject
{
public:
    ViewController( ViewObject& viewObject );

    virtual void Tick() override;

    void Move( const Vec3& delta );
    void Rotate();

private:
    ViewObject& _viewObject;

    bool _isLocked;

    IVec2 _mousePositionDelta;
    IVec2 _prevMousePosition;
};
