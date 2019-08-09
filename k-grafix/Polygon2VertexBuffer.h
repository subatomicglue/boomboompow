#ifndef POLYGON2VERTEXBUFFER_H
#define POLYGON2VERTEXBUFFER_H

#include <vector>
#include "Gfx.h"
#include "triangulate.h"


// convert arbitrary polygon to vertex buffer.
inline void Polygon2VertexBuffer( std::vector<char>&vb,
                           const FvF* fvf,
                           const std::vector<float >& verts,
                           const std::vector<float >& tverts )
{
   int fvfV = FindIndex( FvF::VERTEX, fvf ),
         fvfC = FindIndex( FvF::COLOR, fvf ),
         fvfT = FindIndex( FvF::TEXCOORD, fvf ),
         fvfN = FindIndex( FvF::NORMAL, fvf );

   assert( verts.size() != 0 && verts.size() == tverts.size() );

   // triangulate the polygon
   Vector2dVector polygon;
   for (int x = 0; x < verts.size(); x += 2)
   {
      Vector2d v2d;
      v2d.v[0]  = verts[x + 0];
      v2d.v[1]  = verts[x + 1];
      v2d.tv[0] = tverts[x + 0];
      v2d.tv[1] = tverts[x + 1];
      polygon.push_back( v2d );
   }
   Vector2dVector tris;
   Triangulate::Process(polygon, tris);

   // dump tris into mesh vertex buffer, per the FVF above.
   vb.resize( tris.size() * fvf[0].stride );
   for (int x = 0; x < tris.size(); ++x)
   {
      if (fvfV != -1)
      {
         float* vert = (float*)&vb[x * fvf[fvfV].stride + fvf[fvfV].start];
         assert( 2 <= fvf[fvfV].cnt );
         for (int e = 0; e < 2; ++e)
            vert[e] = tris[x].v[e];
         for (int e = 2; e < fvf[fvfV].cnt; ++e)
            vert[e] = 0.0f;
      }

      if (fvfC != -1)
      {
         float* color = (float*)&vb[x * fvf[fvfC].stride + fvf[fvfC].start];
         for (int e = 0; e < fvf[fvfC].cnt; ++e)
            color[e] = 1.0f;
      }

      if (fvfT != -1)
      {
         float* texc = (float*)&vb[x * fvf[fvfT].stride + fvf[fvfT].start];
         for (int e = 0; e < fvf[fvfT].cnt; ++e)
            texc[e] = tris[x].tv[e];
      }

      if (fvfN != -1)
      {
         float* norm = (float*)&vb[x * fvf[fvfN].stride + fvf[fvfN].start];
         for (int e = 0; e < fvf[fvfN].cnt; ++e)
            norm[e] = 0.0f;
      }
   }
}


#endif


