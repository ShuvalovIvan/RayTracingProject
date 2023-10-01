#pragma once

#include <iostream>
#include <string>

#define LOG_AND_THROW(e)  std::cerr << "Throw " << (e).what() << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
   throw e;

// Top level class to setup the Graphical Environment.
class GraphicalEnvironment {
protected:
    GraphicalEnvironment() {}
    virtual ~GraphicalEnvironment() {}

    GraphicalEnvironment(const GraphicalEnvironment&) = delete;
    GraphicalEnvironment& operator=(const GraphicalEnvironment&) = delete;

    virtual void init() = 0;
    virtual void load_preconfigured_shapes() = 0;

    virtual void add_texture(const std::string &file) = 0;

    virtual void start_interactive_loop() = 0;
};
