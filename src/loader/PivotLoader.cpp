#include "PivotLoader.h"

#include <map>

#include "util/Util.h"

#include "loader_util.h"

namespace {
    const std::string PIVOT_ORIGIN{ "o" };
    const std::string PIVOT_MIDDLE{ "M" };
    const std::string PIVOT_TOP{ "T" };
    const std::string PIVOT_BOTTOM{ "B" };
    const std::string PIVOT_LEFT{ "L" };
    const std::string PIVOT_RIGHT{ "R" };

    std::map<std::string, PivotAlignment> m_pivots;

    std::map<std::string, PivotAlignment>& getPivotMapping() {
        if (m_pivots.empty()) {
            m_pivots[PIVOT_ORIGIN] = PivotAlignment::origin;
            m_pivots[PIVOT_MIDDLE] = PivotAlignment::middle;
            m_pivots[PIVOT_TOP] = PivotAlignment::top;
            m_pivots[PIVOT_BOTTOM] = PivotAlignment::bottom;
            m_pivots[PIVOT_LEFT] = PivotAlignment::left;
            m_pivots[PIVOT_RIGHT] = PivotAlignment::right;
        }
        return m_pivots;
    }

    PivotAlignment resolvePivot(const std::string& p) {
        const auto& mapping = getPivotMapping();
        const auto& it = mapping.find(p);
        if (it != mapping.end()) return it->second;
        return PivotAlignment::origin;
    }

    void readAlignment(
        const loader::DocNode& node,
        PivotPoint& pivot)
    {
        auto vec = readStringVector(node, 3);

        for (auto& p : vec) {
            p = util::toUpper(p);
        }

        if (vec.size() == 1) {
            vec.push_back(vec[0]);
            vec.push_back(vec[0]);
        }
        else {
            while (vec.size() < 3) {
                vec.push_back(PIVOT_ORIGIN);
            }
        }

        for (int i = 0; i < 3; i++) {
            pivot.m_alignment[i] = resolvePivot(vec[i]);
        }
    }
}

namespace loader {
    PivotPoint PivotLoader::load(const loader::DocNode& node) const
    {
        PivotPoint pivot;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "alignment") {
                readAlignment(v, pivot);
            }
            else if (k == "offset") {
                pivot.m_offset = readVec3(v);
            }
        }
        return pivot;
    }
}
