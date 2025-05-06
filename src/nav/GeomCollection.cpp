#include "GeomCollection.h"

#include "InputGeom.h"

namespace nav
{
    GeomCollection::GeomCollection()
    { }

    GeomCollection::~GeomCollection()
    { }

    void GeomCollection::addInput(std::unique_ptr<nav::InputGeom> input)
    {
        m_dirty = true;
        m_inputs.push_back(std::move(input));
    }

    void GeomCollection::build()
    {
        if (!m_dirty) return;

        m_totalTriCount = 0;
        for (auto& input : m_inputs) {
            input->build();
            m_maxTriCount = std::max(m_maxTriCount, input->getTriCount());
        }

        m_dirty = true;
    }
}
