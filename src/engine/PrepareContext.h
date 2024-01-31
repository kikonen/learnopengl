#pragma once

class Registry;

//
// Context for doing prepare, without rendering
//
struct PrepareContext final {
public:
    PrepareContext(
        Registry* registry);

public:
    Registry* const m_registry;
};
