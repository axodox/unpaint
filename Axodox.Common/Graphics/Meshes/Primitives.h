#pragma once
#include "IndexedMesh.h"

namespace Axodox::Graphics
{
  IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreateQuad(float size = 1.f);
  IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreateCube(float size = 1.f, bool useSameTextureOnAllFaces = true);
  IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreatePlane(float size, DirectX::XMUINT2 subdivisions);
}