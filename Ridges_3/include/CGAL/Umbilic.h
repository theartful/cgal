#ifndef _UMBILIC_H_
#define _UMBILIC_H_

#include <list>
#include <vector>
#include <math.h>
#include <CGAL/basic.h>
#include <CGAL/PolyhedralSurf_neighbors.h>

//#include <CGAL/number_utils_classes.h>


CGAL_BEGIN_NAMESPACE

enum Umbilic_type { UMBILIC_NON_GENERIC = 0, UMBILIC_WEDGE, UMBILIC_TRISECTOR};

//-------------------------------------------------------------------
//Umbilic : stores umbilic data, its location given by a vertex, its
//type and a circle of edges bording a disk containing the vertex
//------------------------------------------------------------------
template < class Poly >
class Umbilic
{
 public:
  typedef typename Poly::Vertex_handle    Vertex_handle;
  typedef typename Poly::Halfedge_handle  Halfedge_handle;
  typedef typename Poly::Traits::Vector_3 Vector_3;
  
  //contructor
  Umbilic(Vertex_handle v_init,
	  std::list<Halfedge_handle> contour_init); 
  //access fct
  const Vertex_handle vertex() const { return v;}
  const Umbilic_type umbilic_type() const { return umb_type;}
  Umbilic_type& umbilic_type() { return umb_type;}
  const std::list<Halfedge_handle>& contour_list() const { return contour;}
  std::list<Halfedge_handle>& contour_list() { return contour;}

 protected:
  Vertex_handle v;
  Umbilic_type umb_type;
  std::list<Halfedge_handle> contour;
};

//contructor
template <class Poly>
Umbilic<Poly>::
Umbilic(Vertex_handle v_init,
	std::list<Halfedge_handle> contour_init) 
  : v(v_init), contour(contour_init) {} 


template <class Poly>
std::ostream& 
operator<<(std::ostream& out_stream, const Umbilic<Poly>& umbilic)
{
  out_stream << "Umbilic at location (" << umbilic.vertex()->point() << ") of type " ;
  switch (umbilic.umbilic_type())
    {
    case CGAL::UMBILIC_NON_GENERIC: out_stream << "non generic" << std::endl; break;
    case CGAL::UMBILIC_WEDGE: out_stream << "wedge" << std::endl; break;
    case CGAL::UMBILIC_TRISECTOR: out_stream << "trisector" << std::endl; break;
    default : out_stream << "Something wrong occured for sure..." << std::endl; break;
    }
  return out_stream;
}
//---------------------------------------------------------------------------
//Umbilic_approximation : enable computation of umbilics of a
//TriangularPolyhedralSurface. It uses the class
//T_PolyhedralSurf_neighbors to compute topological disk patches
//around vertices
//--------------------------------------------------------------------------
template < class Poly, class OutputIt, class Vertex2FTPropertyMap, class Vertex2VectorPropertyMap >
  class Umbilic_approximation
{
 public:
  typedef typename Poly::Traits::FT       FT;
  typedef typename Poly::Traits::Vector_3 Vector_3;
  typedef typename Poly::Vertex_handle    Vertex_handle;
  typedef typename Poly::Halfedge_handle  Halfedge_handle;
  typedef typename Poly::Facet_handle     Facet_handle;
  typedef typename Poly::Facet_iterator   Facet_iterator;
  typedef typename Poly::Vertex_iterator  Vertex_iterator;
  typedef Umbilic<Poly> Umbilic;

  //constructor : sets propertymaps and the poly_neighbors
  Umbilic_approximation(Poly &P, 
			Vertex2FTPropertyMap vertex2k1_pm, Vertex2FTPropertyMap vertex2k2_pm,
			Vertex2VectorPropertyMap vertex2d1_pm, Vertex2VectorPropertyMap vertex2d2_pm);
  //identify umbilics as vertices minimizing the function k1-k2 on
  //their patch and for which the index is not 0. We avoid
  //potential umbilics whose contours touch the border.
  OutputIt compute(OutputIt it, FT size);

 protected:
  Poly* P;
  typedef T_PolyhedralSurf_neighbors<Poly> Poly_neighbors;
  Poly_neighbors* poly_neighbors;

  CGAL::Abs<FT> cgal_abs;
  To_double<FT> To_double;

  //Property maps
  Vertex2FTPropertyMap k1, k2;
  Vertex2VectorPropertyMap d1, d2;

  // index: following CW the contour, we choose an orientation for the
  // max dir of an arbitrary starting point, the max dir field is
  // oriented on the next point so that the scalar product of the
  // consecutive vectors is positive.  Adding all the angles between
  // consecutive vectors around the contour gives ~ -/+180 for a
  // wedge/trisector, ~ 0 gives a false umbilic, everything else gives
  // a non_generic umbilic.
  int compute_type(Umbilic& umb);
};

template < class Poly, class OutputIt, class Vertex2FTPropertyMap, class Vertex2VectorPropertyMap >
  Umbilic_approximation< Poly, OutputIt, Vertex2FTPropertyMap, Vertex2VectorPropertyMap >::
Umbilic_approximation(Poly &P, 
		      Vertex2FTPropertyMap vertex2k1_pm, Vertex2FTPropertyMap vertex2k2_pm,
		      Vertex2VectorPropertyMap vertex2d1_pm, Vertex2VectorPropertyMap vertex2d2_pm)
  : P(&P), k1(vertex2k1_pm), k2(vertex2k2_pm), 
    d1(vertex2d1_pm), d2(vertex2d2_pm)
{
  //check that the mesh is a triangular one.
  Facet_iterator itb = P.facets_begin(), ite = P.facets_end();
  for(;itb!=ite;itb++) CGAL_precondition( itb->is_triangle() );

  poly_neighbors = new Poly_neighbors(P);
}

template < class Poly, class OutputIt, class Vertex2FTPropertyMap, class Vertex2VectorPropertyMap >
  OutputIt Umbilic_approximation< Poly, OutputIt, Vertex2FTPropertyMap, Vertex2VectorPropertyMap >::
compute(OutputIt umbilics_it, FT size)
{
  CGAL_precondition( size >= 1 );
  
  std::vector<Vertex_handle> vces;
  std::list<Halfedge_handle> contour;
  FT umbilicEstimatorVertex, umbilicEstimatorNeigh;
  
  bool is_umbilic = true;

  //MAIN loop on P vertices
  Vertex_iterator itb = P->vertices_begin(), ite = P->vertices_end();
  for (;itb != ite; itb++) {
    Vertex_handle vh = itb;
    umbilicEstimatorVertex = cgal_abs(k1[vh]-k2[vh]);
    //reset vector, list and bool
    vces.clear();
    contour.clear();
    is_umbilic = true;
    //the size of neighbourhood is (size * OneRingSize)
    poly_neighbors->compute_neighbors(vh, vces, contour, size);
    
    
    // avoid umbilics whose contours touch the border (Note may not be
    // desirable?)
    typename std::list<Halfedge_handle>::iterator itb_cont = contour.begin(),
      ite_cont = contour.end();
    for (; itb_cont != ite_cont; itb_cont++)
      if ( (*itb_cont)->is_border() ) {is_umbilic = false; continue;}
    if (is_umbilic == false) continue;
    
    //is v an umbilic?
    //a priori is_umbilic = true, and it switches to false as soon as a 
    //  neigh vertex has a lower umbilicEstimator value
    typename std::vector<Vertex_handle>::iterator itbv = vces.begin(),
      itev = vces.end();
    itbv++;
    for (; itbv != itev; itbv++)
      {	umbilicEstimatorNeigh = cgal_abs( k1[*itbv] - k2[*itbv] );
	if ( umbilicEstimatorNeigh < umbilicEstimatorVertex ) 
	  {is_umbilic = false; break;}
      }
    if (is_umbilic == false) continue;
    
    //v is an umbilic (wrt the min of k1-k2), compute the index. If
    //the index is not 0 then we have actually an umbilic which is output
    Umbilic*  cur_umbilic = new Umbilic(vh, contour);
    if (compute_type(*cur_umbilic) != 0)  *umbilics_it++ = cur_umbilic;
  }
  return umbilics_it;
}

template < class Poly, class OutputIt, class Vertex2FTPropertyMap, class Vertex2VectorPropertyMap >
  int Umbilic_approximation< Poly, OutputIt, Vertex2FTPropertyMap, Vertex2VectorPropertyMap >::
compute_type(Umbilic& umb)
{
  Vector_3 dir, dirnext, normal;
  double cosinus, angle=0, angleSum=0;
  const double  pi=3.141592653589793;
  Vertex_handle v;
  typename std::list<Halfedge_handle>::iterator itb = umb.contour_list().begin(),
    itlast = --umb.contour_list().end();
  v = (*itb)->vertex();

  dir = d1[v];
  normal = CGAL::cross_product(d1[v], d2[v]);

  //sum angles along the contour
  do{
    itb++;
    v=(*itb)->vertex();
    dirnext = d1[v];
    cosinus = To_double(dir*dirnext);
    if (cosinus < 0) {dirnext = dirnext*(-1); cosinus *= -1;}
    if (cosinus>1) cosinus = 1;
    //orientation of (dir, dirnext, normal)
    if ( (dir * CGAL::cross_product(dirnext, normal)) > 0) angle = acos(cosinus);
    else angle = -acos(cosinus);
    angleSum += angle;
    dir = dirnext;
    normal = CGAL::cross_product(d1[v], d2[v]);
  }
  while (itb != (itlast));
  
  //angle (v_last, v_0)
  v=(*umb.contour_list().begin())->vertex();
   dirnext = d1[v];
   cosinus = To_double(dir*dirnext);
  if (cosinus < 0) {dirnext = dirnext*(-1); cosinus *= -1;}
  if (cosinus>1) cosinus = 1;
  if ( (dir * CGAL::cross_product(dirnext, normal)) > 0) angle = acos(cosinus);
  else angle = -acos(cosinus);
  angleSum += angle;

  if ((angleSum > (pi/2)) && (angleSum < (3*pi/2))) umb.umbilic_type() = UMBILIC_TRISECTOR ;
  else if ((angleSum < (-pi/2)) && (angleSum > (-3*pi/2))) umb.umbilic_type() = UMBILIC_WEDGE;
  else if ((angleSum <= (pi/2)) && (angleSum >= (-pi/2))) return 0;//is not considered as an umbilic
  else umb.umbilic_type() = UMBILIC_NON_GENERIC;
  return 1;
}

CGAL_END_NAMESPACE

#endif
