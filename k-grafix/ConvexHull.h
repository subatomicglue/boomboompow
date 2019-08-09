#ifndef CONVEXHULL_H
#define CONVEXHULL_H

#include "AlignAllocator.h"
#include "EASTL/list.h"

class ConvexHull
{
public:
   struct Vector2
   {
      float X;
      float Y;
      Vector2() {}
      Vector2(float x, float y)
      {
         X = x;
         Y = y;
      }

      bool Equals(Vector2 ob) const
      {
         Vector2 c = (Vector2)ob;
         return X == c.X && Y == c.Y;
      }
   };

   struct Vector3
   {
      float X;
      float Y;
      float Z;

      Vector3(float x, float y, float Z)
      {
         X = x;
         Y = y;
         Z = Z;
      }
   };

   struct SuperPoint
   {
      Vector2 P;
      int ID;

      SuperPoint(Vector2 p, int id)
      {
         P = p;
         ID = id;
      }
   };

   struct Segment
   {
      Vector2 p;
      Vector2 q;

      bool contains(const SuperPoint& point) const
      {
         if (p.Equals(point.P) || q.Equals(point.P))
            return true;
         return false;
      }
   };




   typedef vmVec4 PointF;
   typedef eastl::list<Vector2, align_allocator<16> > PointList;
   typedef eastl::list<Segment, align_allocator<16> > EdgeList;
   typedef eastl::list<SuperPoint, align_allocator<16> > SuperPointList;
   SuperPointList m_pointsList;
   EdgeList m_segments;
   int m_pID;

   ConvexHull( const PointList& points )
   {
      m_pID = 0;
      for (PointList::const_iterator it = points.begin(); it != points.end(); ++it)
      {
         AddSuperPoint( *it );
      }
   }

   void AddSuperPoint(Vector2 point)
   {
      m_pID++;
      m_pointsList.push_back(SuperPoint(point, m_pID));
   }

   EdgeList& Calculate( EdgeList& returnValue )
   {
      if (m_pointsList.size() > 0)
      {
         int countOfPoints = m_pointsList.size();
         InitOrderdPoints();
         //printf("InitOrderdPoints. Count of points %d.", countOfPoints);

         Compute( returnValue );
         //printf("Finished calculating convex hull. Count of points %d", countOfPoints);
      }
      return returnValue;
   }

private:
   EdgeList& Compute(EdgeList& edges)
   {
      for (EdgeList::iterator it = m_segments.begin();
           it != m_segments.end(); ++it)
      {
         if (isEdge( m_pointsList, *it ))
            edges.push_back( *it );
      }
      return edges;
   }

   /// Iterates over all processingPoint and identify if current segment is edge.
   bool isEdge(const SuperPointList& processingPoints, const Segment& edge)
   {
      for (SuperPointList::const_iterator it = processingPoints.begin();
           it != processingPoints.end(); ++it)
      {
         //ProcessingPoints will be the points that are not in the current segment
         if (!edge.contains( *it ))
         {
            if (isLeft(edge, it->P))
            {
               return false;
            }
         }
      }
      return true;
   }

   bool isLeft(const Segment& segment, Vector2 r)
   {
      float D = 0;
      float px, py, qx, qy, rx, ry = 0;
      //The determinant
      // | 1 px py |
      // | 1 qx qy |
      // | 1 rx ry |
      //if the determinant result is positive then the point is left of the segment
      px = segment.p.X;
      py = segment.p.Y;
      qx = segment.q.X;
      qy = segment.q.Y;
      rx = r.X;
      ry = r.Y;

      D = ((qx * ry) - (qy * rx)) - (px * (ry - qy)) + (py * (rx - qx));

      if (D <= 1)
         return false;

      return true;
   }

   void InitOrderdPoints()
   {
      //Initialize all possible segments from the picked points
      for (SuperPointList::iterator i = m_pointsList.begin();
           i != m_pointsList.end(); ++i)
      {
         for (SuperPointList::iterator j = m_pointsList.begin();
           j != m_pointsList.end(); ++j)
         {
            if (i != j)
            {
               Segment op;
               Vector2 p1 = i->P;
               Vector2 p2 = j->P;
               op.p = p1;
               op.q = p2;

               m_segments.push_back(op);
            }
         }
      }
   }

public:
   static void test()
   {
      printf( "Running ConvexHull test... " );
      fflush( stdout );
      PointList points;
      points.push_back(Vector2(0, 0));
      points.push_back(Vector2(10,0));
      points.push_back(Vector2(10,10));
      points.push_back(Vector2(0,10));
      points.push_back(Vector2(5, 5));
      points.push_back(Vector2(5, 10));
      points.push_back(Vector2(6, 10));

      ConvexHull convexHull( points );
      EdgeList edges;
      convexHull.Calculate(edges);
      for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it)
      {
         Segment& item = *it;
         if (((int)item.p.X == 5 && (int)item.p.Y == 5)
             || ((int)item.q.X == 5 && (int)item.q.Y == 5))
         {
            assert("Convex Hull is not really working.");
         }
      }
      printf( "done.\n" );
   }

};

#endif

