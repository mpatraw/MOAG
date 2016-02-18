
#ifndef ENTITY_HPP
#define ENTITY_HPP

// Base class for all entitys.
class entity {
public:
    virtual ~entity() {}

    // Used for deletion.
    bool alive = true;
};

// Base class for all components. Every component has a parent.
class component {
public:
    component(entity *parent_) : parent{parent_} {}
    virtual ~component() {}

    entity *const parent;
};

#endif
