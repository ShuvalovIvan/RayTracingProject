#pragma once

#include <string>

// Top level class to setup the Graphical Environment.
class GraphicalEnvironment {
protected:
    GraphicalEnvironment() {}
    virtual ~GraphicalEnvironment() {}

    GraphicalEnvironment(const GraphicalEnvironment&) = delete;
    GraphicalEnvironment& operator=(const GraphicalEnvironment&) = delete; 

    virtual void init() = 0;
    virtual void load_preconfigured_shapes() = 0;
};
