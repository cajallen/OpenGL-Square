//UMN CSCI 5607 2D Geometry Library Homework [HW0]

#ifndef GEOM_LIB_H
#define GEOM_LIB_H

#include "pga.h"

//Displace a point p on the direction d
//The result is a point
Point2D move(Point2D p, Dir2D d){
  return p + d;
}

//Compute the displacement vector between points p1 and p2
//The result is a direction 
Dir2D displacement(Point2D p1, Point2D p2){
  Dir2D ret = p2 - p1;
  return ret;
}

//Compute the distance between points p1 and p2
//The result is a scalar 
float dist(Point2D p1, Point2D p2){
  return displacement(p1, p2).magnitude();
}

//Compute the perpendicular distance from the point p the the line l
//The result is a scalar 
float dist(Line2D l, Point2D p){
  return vee(l.normalized(), p);
}

//Compute the perpendicular distance from the point p the the line l
//The result is a scalar 
float dist(Point2D p, Line2D l){
  return vee(l.normalized(), p);
}

//Compute the intersection point between lines l1 and l2
//You may assume the lines are not parallel
//The results is a a point that lies on both lines
Point2D intersect(Line2D l1, Line2D l2){
  return wedge(l1, l2);
}

//Compute the line that goes through the points p1 and p2
//The result is a line 
Line2D join(Point2D p1, Point2D p2){
  return vee(p1, p2);
}

//Compute the projection of the point p onto line l
//The result is the closest point to p that lies on line l
Point2D project(Point2D p, Line2D l){
  return dot(l, p)*l;
}

//Compute the projection of the line l onto point p
//The result is a line that lies on point p in the same direction of l
Line2D project(Line2D l, Point2D p){
  return dot(l, p)*p;
}

//Compute the angle point between lines l1 and l2
//You may assume the lines are not parallel
//The results is a scalar
float angle(Line2D l1, Line2D l2){
  return l1.angle() - l2.angle();
}

//Compute if the line segment p1->p2 intersects the line segment a->b
//The result is a boolean
bool segmentSegmentIntersect(Point2D p1, Point2D p2, Point2D a, Point2D b){
  Line2D l1 = vee(p1, p2);
  Line2D l2 = vee(a, b);

  bool l1_intersects = std::signbit(vee(l1, a)) != std::signbit(vee(l1, b));
  bool l2_intersects = std::signbit(vee(l2, p1)) != std::signbit(vee(l2, p2));

  return l1_intersects && l2_intersects;
}

//Compute if the point p lies inside the polygon
//Works for both clockwise and counterclockwise, but does NOT confirm convexity
bool pointInConvexPolygon(Point2D p, std::vector<Line2D> lines){
	// initialize `right` with first line, iterate through the rest and exit if any don't match
	bool right = false;
	bool initialized = false;
	for (Line2D line : lines) {
		if (!initialized) {
			initialized = true;
			right = std::signbit(vee(p, line));
		} else {
			if (right != std::signbit(vee(p, line))) {
				return false;
			}
		}
	}
	return true;
}

//Compute the area of the triangle t1,t2,t3
//The result is a scalar
float areaTriangle(Point2D t1, Point2D t2, Point2D t3){
  return 0.5 * vee(vee(t1, t2), t3);
}

float distToSegment(Point2D p, Point2D p1, Point2D p2) {
    float segment_dist = dist(p1, p2);
    Line2D line = join(p1, p2);

    float correct_dist = 0;
    float project_dist = dist(p, line);
    Point2D projected_point = project(p, line);

    float dist1 = dist(projected_point, p1);
    float dist2 = dist(projected_point, p2);

    if (dist1 > segment_dist) {
        correct_dist = fmin(dist1, dist2);
    }
    else if (dist2 > segment_dist) {
        correct_dist = dist1;
    }

    return sqrt(pow(project_dist, 2) + pow(correct_dist, 2));
}

//Compute the distance from the point p to the triangle t1,t2,t3 as defined 
//by it's distance from the edge closest to p.
//The result is a scalar
float pointEdgeDist(Point2D p, std::vector<Point2D> line_segments){
  float min = -1.0f;
  for (int i = 0; i < line_segments.size(); i++) {
	  float new_dist = fabs(distToSegment(p, line_segments[i], line_segments[(i+1)%line_segments.size()]));
	  min = new_dist < min || min == -1.0f ? new_dist : min;
  }
  return min;
}

//Compute the distance from the point p to the closest point
//The result is a scalar
float pointCornerDist(Point2D p, std::vector<Point2D> points){
  float min = -1.0f;
  for (Point2D point : points) {
	  float new_dist = dist(p, point);
	  min = new_dist < min || min == -1.0f ? new_dist : min;
  }
  return min;
}

//Compute if the quad (p1,p2,p3,p4) is convex.
//Your code should work for both clockwise and counterclockwise windings
//The result is a boolean
bool isConvex_Quad(Point2D p1, Point2D p2, Point2D p3, Point2D p4){
  Line2D l1 = join(p1, p2);
  Line2D l2 = join(p2, p3);
  Line2D l3 = join(p3, p4);
  Line2D l4 = join(p4, p1);

  bool right_1 = std::signbit(vee(p3, l1));
  bool right_2 = std::signbit(vee(p4, l2));
  bool right_3 = std::signbit(vee(p1, l3));
  bool right_4 = std::signbit(vee(p2, l4));

  return (right_1 == right_2) && (right_2 == right_3) && (right_3 == right_4);
}

//Compute the reflection of the point p about the line l
//The result is a point
Point2D reflect(Point2D p, Line2D l){
  return l*p*l;
}

//Compute the reflection of the line d about the line l
//The result is a line
Line2D reflect(Line2D d, Line2D l){
  return l*d*l;
}

#endif