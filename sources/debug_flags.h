#pragma once

#include "practical_entities.h"

class DebugFlags : public PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    DebugFlags() {}
    ~DebugFlags() {}
    //--------< ä÷êî >--------//
    /*åƒÇ—èoÇµÇÕFrameworkÇÃÇ›*/
    void update();
    //--------<getter>--------//
    static const bool get_perspective_switching() { return perspective_switching; }
    static const bool get_wireframe_switching() { return wireframe_switching; }
    static const bool get_debug_2D_switching() { return debug_2D_switching; }
private:
    //--------< ïœêî >--------//
    static bool perspective_switching;
    static bool wireframe_switching;
    static bool debug_2D_switching;
};