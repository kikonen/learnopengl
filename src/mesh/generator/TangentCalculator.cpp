#include "TangentCalculator.h"

#include <limits>

#include "mesh/VaoMesh.h"

namespace {
    inline bool is_special_float(float in) {
        return std::isnan(in) || std::isinf(in);
    }

    inline float get_qnan() {
        return std::numeric_limits<float>::quiet_NaN();
    }
}

namespace mesh {
    // ------------------------------------------------------------------------------------------------
    // Calculates tangents and bi-tangents for the given mesh
    //
    // @see Assimp code/PostProcessing/CalcTangentsProcess.cpp
    //
    void TangentCalculator::calculateTangents(mesh::VaoMesh& mesh) {
        // we assume that the mesh is still in the verbose vertex format where each face has its own set
        // of vertices and no vertices are shared between faces. Sadly I don't know any quick test to
        // assert() it here.
        // assert( must be verbose, dammit);

        const float angleEpsilon = 0.9999f;

        auto& vertices = mesh.m_vertices;
        // each 3 indeces form face
        const auto& indeces = mesh.m_indeces;

        std::vector<bool> vertexDone(vertices.size(), false);
        const float qnan = get_qnan();

        // calculate the tangent and bitangent for every face
        for (unsigned int a = 0; a < indeces.size(); a += 3) {
            glm::uvec3 face{ indeces[a], indeces[a + 1], indeces[a + 2] };

            // triangle or polygon... we always use only the first three indices. A polygon
            // is supposed to be planar anyways....
            // FIXME: (thom) create correct calculation for multi-vertex polygons maybe?
            const unsigned int p0 = face[0], p1 = face[1], p2 = face[2];

            auto& v0 = vertices[p0];
            auto& v1 = vertices[p1];
            auto& v2 = vertices[p2];

            // position differences p1->p2 and p1->p3
            const glm::vec3 v = v1.pos - v0.pos;
            const glm::vec3 w = v2.pos - v0.pos;

            // texture offset p1->p2 and p1->p3
            float sx = v1.texCoord.x - v0.texCoord.x, sy = v1.texCoord.y - v0.texCoord.y;
            float tx = v2.texCoord.x - v0.texCoord.x, ty = v2.texCoord.y - v0.texCoord.y;
            float dirCorrection = (tx * sy - ty * sx) < 0.0f ? -1.0f : 1.0f;

            // when t1, t2, t3 in same position in UV space, just use default UV direction.
            if (sx * ty == sy * tx) {
                sx = 0.0;
                sy = 1.0;
                tx = 1.0;
                ty = 0.0;
            }

            // tangent points in the direction where to positive X axis of the texture coord's would point in model space
            // bitangent's points along the positive Y axis of the texture coord's, respectively
            glm::vec3 tangent, bitangent;
            tangent.x = (w.x * sy - v.x * ty) * dirCorrection;
            tangent.y = (w.y * sy - v.y * ty) * dirCorrection;
            tangent.z = (w.z * sy - v.z * ty) * dirCorrection;
            bitangent.x = (w.x * sx - v.x * tx) * dirCorrection;
            bitangent.y = (w.y * sx - v.y * tx) * dirCorrection;
            bitangent.z = (w.z * sx - v.z * tx) * dirCorrection;

            // store for every vertex of that face
            for (unsigned int b = 0; b < 3; ++b) {
                unsigned int p = face[b];
                auto& v = vertices[p];

                // project tangent and bitangent into the plane formed by the vertex' normal
                glm::vec3 localTangent = tangent - v.normal * (tangent * v.normal);
                glm::vec3 localBitangent = bitangent - v.normal * (bitangent * v.normal);
                localTangent = glm::normalize(localTangent);
                localBitangent = glm::normalize(localBitangent);

                //// reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
                //bool invalid_tangent = is_special_float(localTangent.x) || is_special_float(localTangent.y) || is_special_float(localTangent.z)
                //    || (-0.5f < localTangent.x && localTangent.x < 0.5f && -0.5f < localTangent.y && localTangent.y < 0.5f && -0.5f < localTangent.z && localTangent.z < 0.5f);

                //bool invalid_bitangent = is_special_float(localBitangent.x) || is_special_float(localBitangent.y) || is_special_float(localBitangent.z)
                //    || (-0.5f < localBitangent.x && localBitangent.x < 0.5f && -0.5f < localBitangent.y && localBitangent.y < 0.5f && -0.5f < localBitangent.z && localBitangent.z < 0.5f);

                //if (invalid_tangent != invalid_bitangent) {
                //    if (invalid_tangent) {
                //        localTangent = v.normal ^ localBitangent;
                //        localTangent = glm::normalize(localTangent);
                //    }
                //    else {
                //        localBitangent = localTangent ^ v.normal;
                //        localBitangent = glm::normalize(localBitangent);
                //    }
                //}

                // and write it into the mesh.
                v.tangent = localTangent;
                //v.bitangent = localBitangent;
            }
        }

        //std::vector<unsigned int> verticesFound;

        //const float fLimit = std::cos(configMaxAngle);
        //std::vector<unsigned int> closeVertices;

        //// in the second pass we now smooth out all tangents and bitangents at the same local position
        //// if they are not too far off.
        //for (unsigned int a = 0; a < pMesh->mNumVertices; a++) {
        //    if (vertexDone[a])
        //        continue;

        //    const glm::vec3& origPos = pMesh->mVertices[a];
        //    const glm::vec3& origNorm = pMesh->mNormals[a];
        //    const glm::vec3& origTang = pMesh->mTangents[a];
        //    const glm::vec3& origBitang = pMesh->mBitangents[a];
        //    closeVertices.resize(0);

        //    // find all vertices close to that position
        //    vertexFinder->FindPositions(origPos, posEpsilon, verticesFound);

        //    closeVertices.reserve(verticesFound.size() + 5);
        //    closeVertices.push_back(a);

        //    // look among them for other vertices sharing the same normal and a close-enough tangent/bitangent
        //    for (unsigned int b = 0; b < verticesFound.size(); b++) {
        //        unsigned int idx = verticesFound[b];
        //        if (vertexDone[idx])
        //            continue;
        //        if (meshNorm[idx] * origNorm < angleEpsilon)
        //            continue;
        //        if (meshTang[idx] * origTang < fLimit)
        //            continue;
        //        if (meshBitang[idx] * origBitang < fLimit)
        //            continue;

        //        // it's similar enough -> add it to the smoothing group
        //        closeVertices.push_back(idx);
        //        vertexDone[idx] = true;
        //    }

        //    // smooth the tangents and bitangents of all vertices that were found to be close enough
        //    glm::vec3 smoothTangent{ 0.f },
        //        smoothBitangent{ 0.f };

        //    for (unsigned int b = 0; b < closeVertices.size(); ++b) {
        //        smoothTangent += meshTang[closeVertices[b]];
        //        smoothBitangent += meshBitang[closeVertices[b]];
        //    }
        //    smoothTangent = glm::normalize(smoothTangent);
        //    smoothBitangent = glm::normalize(smoothBitangent);

        //    // and write it back into all affected tangents
        //    for (unsigned int b = 0; b < closeVertices.size(); ++b) {
        //        meshTang[closeVertices[b]] = smoothTangent;
        //        meshBitang[closeVertices[b]] = smoothBitangent;
        //    }
        //}
    }
}
