#ifndef CGAL_PARABOLA_SEGMENT_2_H
#define CGAL_PARABOLA_SEGMENT_2_H

#ifndef CGAL_REP_CLASS_DEFINED
#error  no representation class defined
#endif  // CGAL_REP_CLASS_DEFINED

#include <CGAL/Point_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/IO/Window_stream.h>
#include <CGAL/Parabola_2.h>

CGAL_BEGIN_NAMESPACE

template < class Point, class Weight, class Line >
class Parabola_segment_2 : public Parabola_2< Point, Weight, Line >
{
  typedef CGAL::Weighted_point< Point, Weight >   Weighted_point;
  typedef double                                  FT;
  typedef CGAL::Point_2< Cartesian<double> >      Point_2;
  typedef CGAL::Segment_2< Cartesian<double> >    Segment_2;
  typedef CGAL::Line_2< Cartesian<double> >       Line_2;

protected:
  Point_2 p1, p2;

public:
  Parabola_segment_2() : Parabola_2< Point, Weight, Line >() {}

  Parabola_segment_2(const Weighted_point &p, const Line &l,
		     const Point &p1, const Point &p2) :
    Parabola_2< Point, Weight, Line >(p, l)
  {
    this->p1 = Point_2(CGAL_NTS to_double(p1.x()),
		       CGAL_NTS to_double(p1.y()));
    this->p2 = Point_2(CGAL_NTS to_double(p2.x()),
		       CGAL_NTS to_double(p2.y()));
  }

  template< class Stream >
  void draw(Stream &W) const
  {
    vector< Point_2 > p;

    double s[2];

    s[0] = t(p1);
    s[1] = t(p2);

    if (CGAL_NTS compare(s[0], s[1]) == LARGER)
      swap< double >(s[0], s[1]);

    p.clear();

    if ( !(CGAL_NTS is_positive(s[0])) &&
	 !(CGAL_NTS is_negative(s[1])) ) {
      double tt;
      int k;

      p.push_back( o );
      k = 1;
      tt = double(-STEP);
      while ( CGAL_NTS compare(tt, s[0]) == LARGER ) {
	p.insert( p.begin(), f(tt) );
	k--;
	tt = -double(k * k) * STEP;
      }
      p.insert( p.begin(), f(s[0]) );

      k = 1;
      tt = double(STEP);
      while ( CGAL_NTS compare(tt, s[1]) == SMALLER ) {
	p.push_back( f(tt) );
	k++;
	tt = double(k * k) * STEP;
      }
      p.push_back( f(s[1]) );
    } else if ( !(CGAL_NTS is_negative(s[0])) &&
		!(CGAL_NTS is_negative(s[1])) ) {
      double tt;
      int k;


      p.push_back( f(s[0]) );

      tt = s[0];
      k = int(CGAL_NTS to_double(CGAL_NTS sqrt(tt / STEP)));

      while ( CGAL_NTS compare(tt, s[1]) == SMALLER ) {
	if ( CGAL_NTS compare(tt, s[0]) != SMALLER )
	  p.push_back( f(tt) );
	k++;
	tt = double(k * k) * STEP;
      }
      p.push_back( f(s[1]) );
    } else {
      double tt;
      int k;

      p.push_back( f(s[1]) );

      tt = s[1];
      k = int(CGAL_NTS to_double(-CGAL_NTS sqrt(-tt / STEP)));

      while ( CGAL_NTS compare(tt, s[0]) == LARGER ) {
	if ( CGAL_NTS compare(tt, s[1]) != LARGER )
	  p.push_back( f(tt) );
	k--;
	tt = -double(k * k) * STEP;
      }
      p.push_back( f(s[0]) );
    }

    for (unsigned int i = 0; i < p.size() - 1; i++) {
      W << Segment_2(p[i], p[i+1]);
    }
  }
};

template< class Stream, class Point, class Weight, class Line >
inline
Stream& operator<<(Stream &s,
		   const Parabola_segment_2< Point, Weight, Line > &P)
{
  P.draw(s);
  return s;
}

CGAL_END_NAMESPACE

#endif // CGAL_PARABOLA_SEGMENT_2_H
