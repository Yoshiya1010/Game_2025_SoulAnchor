#pragma once
#include "btBulletDynamicsCommon.h"
#include "modelRenderer.h"

inline btBvhTriangleMeshShape* CreateTriangleMeshShape(MODEL* model)
{
    btTriangleMesh* triMesh = new btTriangleMesh();

    for (unsigned int i = 0; i < model->CollisionIndices.size(); i += 3)
    {
        auto& p0 = model->CollisionVertices[model->CollisionIndices[i + 0]];
        auto& p1 = model->CollisionVertices[model->CollisionIndices[i + 1]];
        auto& p2 = model->CollisionVertices[model->CollisionIndices[i + 2]];

        triMesh->addTriangle(
            btVector3(p0.x, p0.y, p0.z),
            btVector3(p1.x, p1.y, p1.z),
            btVector3(p2.x, p2.y, p2.z)
        );
    }

    return new btBvhTriangleMeshShape(triMesh, true);
}