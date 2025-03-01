#pragma once

#include <vector>
#include <map>
#include <memory>

#include "pool/NodeHandle.h"

struct Material;
class Registry;

class SelectionRegistry {
public:
    static SelectionRegistry& get() noexcept;

    SelectionRegistry();
    ~SelectionRegistry();

    void clear();

    void prepare(Registry* registry);

    void attachListeners();

    void selectNode(pool::NodeHandle handle, bool append);
    void deselectNode(pool::NodeHandle handle);
    void clearSelection();

    void tagNode(pool::NodeHandle handle, uint8_t tag);
    void untagNode(pool::NodeHandle handle, uint8_t tag);
    void clearTagged();

    size_t getTaggedCount() const noexcept
    {
        return m_tagged.size();
    }

    size_t getSelectedCount() const noexcept
    {
        return m_selected.size();
    }

    inline bool isSelected(const pool::NodeHandle handle) const noexcept
    {
        return std::find(m_selected.begin(), m_selected.end(), handle) != m_selected.end();
    }

    inline bool isTagged(const pool::NodeHandle handle) const noexcept
    {
        return std::find(m_tagged.begin(), m_tagged.end(), handle) != m_tagged.end();
    }

    inline bool isHighlighted(const pool::NodeHandle handle) const noexcept
    {
        const auto& it1 = std::find(m_tagged.begin(), m_tagged.end(), handle);
        const auto& it2 = std::find(m_selected.begin(), m_selected.end(), handle);

        return it1 != m_tagged.begin() || it2 != m_selected.end();
    }

    const Material& getSelectionMaterial() const noexcept;
    void setSelectionMaterial(const Material& material);

    inline uint8_t getTagMaterialIndex() const noexcept
    {
        return m_tagMaterialIndex;
    }

    inline uint8_t getSelectionMaterialIndex() const noexcept
    {
        return m_selectionMaterialIndex;
    }

    void setTagMaterialIndex(uint8_t index)
    {
        m_tagMaterialIndex = index;
    }

    void setSelectionMaterialIndex(uint8_t index)
    {
        m_selectionMaterialIndex = index;
    }

    // @return -1 if no highlight color
    int getHighlightIndex(pool::NodeHandle handle) const noexcept;

private:
    Registry* m_registry{ nullptr };

    std::unique_ptr<Material> m_selectionMaterial;
    std::unique_ptr<Material> m_tagMaterial;

    std::vector<pool::NodeHandle> m_selected;
    std::vector<pool::NodeHandle> m_tagged;

    // special materials are in range 0..255
    uint8_t m_tagMaterialIndex{ 0 };
    uint8_t m_selectionMaterialIndex{ 0 };
};
