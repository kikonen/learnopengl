#pragma once

class Assets;
class Registry;

//
// Context for doing prepare, without rendering
//
struct PrepareContext final {
public:
    PrepareContext(
        Registry* registry);

public:
    const Assets& m_assets;

    Registry* const m_registry;
};
