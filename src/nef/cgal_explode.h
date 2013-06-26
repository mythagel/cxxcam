/* cxxcam - C++ CAD/CAM driver library.
 * Copyright (C) 2013  Nicholas Gill
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * cgal_explode.h
 *
 *  Created on: 2013-06-25
 *      Author: Pierre Alliez
 *      Source: http://cgal-discuss.949826.n4.nabble.com/Parts-of-Polyhedron-after-a-boolean-operations-tp954026p954029.html
 * Minor modifications to build.
 */

#ifndef _CGALEXPLODE_ 
#define _CGALEXPLODE_ 

#include <list>
#include <set>
#include <map> 
#include <stack> 

#include <CGAL/Modifier_base.h> 
#include <CGAL/Polyhedron_incremental_builder_3.h> 

template<class Handle> 
struct less_handle 
{	
  bool operator()(const Handle& h1, 
                  const Handle& h2) const 
  { 
    return &(*h1) < &(*h2); 
  } 
}; 


template <class HDS, class Polyhedron, class Kernel> 
class CModifierExplode : public CGAL::Modifier_base<HDS> 
{ 
private: 
  typedef typename Kernel::Triangle_3 Triangle; 
  typedef typename HDS::Face_handle Face_handle; 
  typedef typename HDS::Vertex_handle Vertex_handle; 
  typedef typename HDS::Halfedge_handle Halfedge_handle; 
  typedef typename Polyhedron::Halfedge_around_facet_circulator F_circulator; 
  typedef typename CGAL::Polyhedron_incremental_builder_3<HDS> builder; 

  typedef typename std::set<Face_handle, less_handle<Face_handle> > Faces; 
  typedef typename std::set<Vertex_handle, less_handle<Vertex_handle> > Vertices; 
  typedef typename std::set<Halfedge_handle, less_handle<Halfedge_handle> > Halfedges; 

  Halfedges& m_halfedges; 
  Halfedge_handle m_seed_halfedge; 

public: 

  // life cycle 
  CModifierExplode(Halfedge_handle seed_halfedge, 
                   Halfedges& halfedges) 
    : m_seed_halfedge(seed_halfedge), 
      m_halfedges(halfedges) 
  { 
  } 
  ~CModifierExplode() {} 

  // make one connected component 
  void operator()( HDS& hds) 
  { 
    Faces faces; 
    Vertices vertices; 
    std::list<Vertex_handle> ordered_vertices; 
    std::map<Vertex_handle,int,less_handle<Vertex_handle> > vertex_map; 
    
    // traverse component and fill sets 
    std::stack<Halfedge_handle> stack; 
    stack.push(m_seed_halfedge); 
    int vertex_index = 0; 
    while(!stack.empty()) 
    { 
      // pop halfedge from queue 
      Halfedge_handle he = stack.top(); 
      stack.pop(); 
      m_halfedges.insert(he); 

      // let's see its incident face 
      if(!he->is_border()) 
      { 
        Face_handle f = he->facet(); 
        if(faces.find(f) == faces.end()) 
          faces.insert(f); 
      } 

      // let's see its end vertex 
      Vertex_handle v = he->vertex(); 
      if(vertices.find(v) == vertices.end()) 
      { 
        vertices.insert(v); 
        ordered_vertices.push_back(v); 
        vertex_map[v] = vertex_index++; 
      } 

      // go and discover component 
      Halfedge_handle nhe = he->next(); 
      Halfedge_handle ohe = he->opposite(); 
      if(m_halfedges.find(nhe) == m_halfedges.end()) 
        stack.push(nhe); 
      if(m_halfedges.find(ohe) == m_halfedges.end()) 
        stack.push(ohe); 
    } 

    builder B(hds,true); 
    B.begin_surface(3,1,6); 

    // add vertices 
    typename std::list<Vertex_handle>::iterator vit; 
    for(vit = ordered_vertices.begin(); 
        vit != ordered_vertices.end(); 
        vit++) 
    { 
      Vertex_handle v = *vit; 
      B.add_vertex(v->point()); 
    } 

    // add facets 
    typename Faces::iterator fit; 
    for(fit = faces.begin(); 
      fit != faces.end(); 
      fit++) 
    { 
      Face_handle f = *fit; 
      B.begin_facet(); 
      F_circulator he = f->facet_begin(); 
      F_circulator end = he; 
      CGAL_For_all(he,end) 
      { 
        Vertex_handle v = he->vertex(); 
        B.add_vertex_to_facet(vertex_map[v]); 
      } 
      B.end_facet(); 
    } 
    B.end_surface(); 
  } 
}; 

template <class Polyhedron, 
          class Kernel, 
          class OutputIterator> 
class Explode_polyhedron 
{ 
public: 
  typedef typename Polyhedron::HalfedgeDS HalfedgeDS; 
  typedef typename Polyhedron::Halfedge_handle Halfedge_handle; 
  typedef typename Polyhedron::Halfedge_iterator Halfedge_iterator; 

public: 
  Explode_polyhedron() {} 
  ~Explode_polyhedron() {} 

public: 
  void run(Polyhedron &polyhedron, 
           OutputIterator out) 
  { 
    std::set<Halfedge_handle,less_handle<Halfedge_handle> > halfedges; 
    typename Polyhedron::Halfedge_iterator he; 
    for(he = polyhedron.halfedges_begin(); 
        he != polyhedron.halfedges_end(); 
        he++) 
    { 
      if(halfedges.find(he) == halfedges.end()) 
      { 
        // adds one component as polyhedron 
        CModifierExplode<HalfedgeDS,Polyhedron,Kernel> modifier(he,halfedges); 
        Polyhedron component; 
        component.delegate(modifier); 
        *out = component; 
        out++; 
      } 
    } 
  } 
}; 

#endif // _CGALEXPLODE_ 
