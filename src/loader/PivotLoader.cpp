#include "PivotLoader.h"

#include <map>

#include "util/Util.h"

#include "loader_util.h"

namespace {
    const std::string PIVOT_ZERO{ "0" };
    const std::string PIVOT_MIDDLE{ "M" };
    const std::string PIVOT_TOP{ "T" };
    const std::string PIVOT_BOTTOM{ "B" };
    const std::string PIVOT_LEFT{ "L" };
    const std::string PIVOT_RIGHT{ "R" };

    std::map<std::string, PivotAlignment> m_pivots;

    std::map<std::string, PivotAlignment>& getPivotMapping() {
        if (m_pivots.empty()) {
            m_pivots[PIVOT_ZERO] = PivotAlignment::zero;
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
        return PivotAlignment::zero;
    }
}

namespace loader {
    PivotPoint PivotLoader::read(const loader::DocNode& node) const
    {
        PivotPoint pivot;

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
                vec.push_back(PIVOT_ZERO);
            }
        }

        for (int i = 0; i < 3; i++) {
            pivot.alignment[i] = resolvePivot(vec[i]);
        }

        return pivot;
    }
}
