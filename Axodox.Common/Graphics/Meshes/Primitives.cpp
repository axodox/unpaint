#include "pch.h"
#include "Primitives.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

namespace Axodox::Graphics
{
  IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreateQuad(float size)
  {
    static const XMBYTEN4 normal{ 0.f, 0.f, 1.f, 1.f };
    size = size / 2;

    IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> result;

    result.Vertices = {
      VertexPositionNormalTexture{ XMFLOAT3{-size, size, 0.f}, normal, XMUSHORTN2{0.f, 0.f} },
      VertexPositionNormalTexture{ XMFLOAT3{-size, -size, 0.f}, normal, XMUSHORTN2{0.f, 1.f} },
      VertexPositionNormalTexture{ XMFLOAT3{size, size, 0.f}, normal, XMUSHORTN2{1.f, 0.f} },
      VertexPositionNormalTexture{ XMFLOAT3{size, -size, 0.f}, normal, XMUSHORTN2{1.f, 1.f} }
    };

    result.Indices = { 0, 1, 2, 1, 3, 2 };
    result.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return result;
  }

  IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreateCube(float size, bool useSameTextureOnAllFaces)
  {
    IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> result;
    size = size / 2;

    //Vertices
    result.Vertices.reserve(24);

    float textureStep = 1.f / 6.f, textureStart = 0.f, textureEnd = 1.f;
    XMBYTEN4 normal;

    normal = XMBYTEN4{ 0.f, 0.f, -1.f, 1.f };
    if (!useSameTextureOnAllFaces) textureEnd = textureStart + textureStep;
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{size, size, -size}, normal, XMUSHORTN2{textureStart, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{size, -size, -size}, normal, XMUSHORTN2{textureEnd, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{-size, -size, -size}, normal, XMUSHORTN2{textureEnd, 0.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{-size, size, -size}, normal, XMUSHORTN2{textureStart, 0.f} });

    normal = XMBYTEN4{ 1.f, 0.f, 0.f, 1.f };
    if (!useSameTextureOnAllFaces) { textureStart = textureEnd; textureEnd = textureStart + textureStep; }
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{size, -size, -size}, normal, XMUSHORTN2{textureStart, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{size, size, -size}, normal, XMUSHORTN2{textureEnd, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{size, size, size}, normal, XMUSHORTN2{textureEnd, 0.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{size, -size, size}, normal, XMUSHORTN2{textureStart, 0.f} });

    normal = XMBYTEN4{ 0.f, 1.f, 0.f, 1.f };
    if (!useSameTextureOnAllFaces) { textureStart = textureEnd; textureEnd = textureStart + textureStep; }
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{size, size, -size}, normal, XMUSHORTN2{textureStart, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{-size, size, -size}, normal, XMUSHORTN2{textureEnd, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{-size, size, size}, normal, XMUSHORTN2{textureEnd, 0.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{size, size, size}, normal, XMUSHORTN2{textureStart, 0.f} });

    normal = XMBYTEN4{ -1.f, 0.f, 0.f, 1.f };
    if (!useSameTextureOnAllFaces) { textureStart = textureEnd; textureEnd = textureStart + textureStep; }
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{-size, size, -size}, normal, XMUSHORTN2{textureStart, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{-size, -size, -size}, normal, XMUSHORTN2{textureEnd, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{-size, -size, size}, normal, XMUSHORTN2{textureEnd, 0.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{-size, size, size}, normal, XMUSHORTN2{textureStart, 0.f} });

    normal = XMBYTEN4{ 0.f, -1.f, 0.f, 1.f };
    if (!useSameTextureOnAllFaces) { textureStart = textureEnd; textureEnd = textureStart + textureStep; }
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{ -size, -size, -size}, normal, XMUSHORTN2{textureStart, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{ size, -size, -size}, normal, XMUSHORTN2{textureEnd, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{ size, -size, size}, normal, XMUSHORTN2{textureEnd, 0.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{ -size, -size, size}, normal, XMUSHORTN2{textureStart, 0.f} });

    normal = XMBYTEN4{ 0.f, 0.f, 1.f, 1.f };
    if (!useSameTextureOnAllFaces) { textureStart = textureEnd; textureEnd = textureStart + textureStep; }
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{ size, -size, size}, normal, XMUSHORTN2{textureStart, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{ size, size, size}, normal, XMUSHORTN2{textureEnd, 1.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{ -size, size, size}, normal, XMUSHORTN2{textureEnd, 0.f} });
    result.Vertices.push_back(VertexPositionNormalTexture{ XMFLOAT3{ -size, -size, size}, normal, XMUSHORTN2{textureStart, 0.f} });

    //Indices
    result.Indices.reserve(36);

    for (auto side = 0u; side < 6u; side++)
    {
      uint32_t index = side * 4u;
      result.Indices.push_back(index);
      result.Indices.push_back(index + 1);
      result.Indices.push_back(index + 2);
      result.Indices.push_back(index + 2);
      result.Indices.push_back(index + 3);
      result.Indices.push_back(index);
    }

    //Topology
    result.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return result;
  }

  IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreatePlane(float size, DirectX::XMUINT2 subdivisions)
  {
    if (subdivisions.x < 2 || subdivisions.y < 2) throw logic_error("Plane size must be at least 2!");
    if (subdivisions.x * subdivisions.y > (uint64_t)numeric_limits<uint32_t>::max() + 1) throw logic_error("Run out of indices!");

    IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> result;

    //Vertices
    float xstep = size / (subdivisions.x - 1), xtexstep = 1.f / (subdivisions.x - 1), xstart = -size / 2.f;
    float ystep = size / (subdivisions.y - 1), ytexstep = 1.f / (subdivisions.y - 1), ystart = -size / 2.f;

    uint32_t vertexCount = subdivisions.x * subdivisions.y;
    result.Vertices.reserve(vertexCount);
    for (uint32_t j = 0; j < subdivisions.y; j++)
    {
      for (uint32_t i = 0; i < subdivisions.x; i++)
      {
        result.Vertices.push_back({
          XMFLOAT3{ xstart + i * xstep, ystart + j * ystep, 0.f },
          XMBYTEN4{ 0.f, 0.f, 1.f, 1.f },
          XMUSHORTN2{ i * xtexstep, 1 - j * ytexstep }
          });
      }
    }

    //Indices
    uint32_t triangleWidth = subdivisions.x - 1, triangleHeight = subdivisions.y - 1;
    uint32_t indexCount = triangleWidth * triangleHeight * 6;
    result.Indices.reserve(indexCount);

    for (uint32_t j = 0; j < triangleHeight; j++)
    {
      for (uint32_t i = 0; i < triangleWidth; i++)
      {
        result.Indices.push_back(j * subdivisions.x + i);
        result.Indices.push_back(j * subdivisions.x + i + 1);
        result.Indices.push_back((j + 1) * subdivisions.x + i);
        result.Indices.push_back(j * subdivisions.x + i + 1);
        result.Indices.push_back((j + 1) * subdivisions.x + i + 1);
        result.Indices.push_back((j + 1) * subdivisions.x + i);
      }
    }

    //Topology
    result.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return result;
  }
}