// 
// Test Suite for geos::triangulate::quadedge::QuadEdge
//
// tut
#include <tut.hpp>
// geos
#include <geos/triangulate/quadedge/Vertex.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
// std
#include <stdio.h>

using namespace geos::triangulate::quadedge;

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_quadedge_data
	{
		test_quadedge_data()
		{
		}
	};

	typedef test_group<test_quadedge_data> group;
	typedef group::object object;

	group test_quadedge_group("geos::triangulate::quadedge::QuadEdge");


	//
	// Test Cases
	//

	// 1 - QuadEdge::connect()
	template<>
	template<>
	void object::test<1>()
	{		 
		Vertex v1(0, 0);
		Vertex v2(0, 1);

		Vertex v3(1, 0);
		Vertex v4(1, 1);

		std::auto_ptr<QuadEdge> q0;
		std::auto_ptr<QuadEdge> r0;
		std::auto_ptr<QuadEdge> s0;

		q0 = QuadEdge::makeEdge(v1, v2);
		r0 = QuadEdge::makeEdge(v3, v4);
		s0 = QuadEdge::connect(*q0, *r0);

		//verify properties ensured by connect()
		//the new edge connects q0->orig() and r0->dest()
		ensure(s0->orig().equals(q0->dest()));
		ensure(s0->dest().equals(r0->orig()));
		//q0, r0, and s0 should have the same left face
		ensure(&q0->lNext() == s0.get());
		ensure(&s0->lNext() == r0.get());

		q0->free();
		r0->free();
		s0->free();
	}

	// 2 - QuadEdge::connect(), causing a loop
	template<>
	template<>
	void object::test<2>()
	{		 
		Vertex v1(0, 0);
		Vertex v2(0, 1);

		Vertex v3(1, 0);
		Vertex v4(1, 1);

		std::auto_ptr<QuadEdge> q0;
		std::auto_ptr<QuadEdge> r0;
		std::auto_ptr<QuadEdge> s0;

		q0 = QuadEdge::makeEdge(v1, v2);
		r0 = QuadEdge::makeEdge(v2, v3);
		s0 = QuadEdge::connect(*q0, *r0);

		//verify properties ensured by connect()
		//the new edge connects q0->orig() and r0->dest()
		ensure(s0->orig().equals(q0->dest()));
		ensure(s0->dest().equals(r0->orig()));
		//q0, r0, and s0 should have the same left face
		ensure(&q0->lNext() == s0.get());
		ensure(&s0->lNext() == r0.get());

		q0->free();
		r0->free();
		s0->free();
	}

	// 3 - QuadEdge::swap()
	template<>
	template<>
	void object::test<3>()
	{		 
		Vertex v1(0, 0);
		Vertex v2(0, 1);

		Vertex v3(1, 0);
		Vertex v4(1, 1);

		std::auto_ptr<QuadEdge> q0;
		std::auto_ptr<QuadEdge> r0;
		std::auto_ptr<QuadEdge> s0;
		std::auto_ptr<QuadEdge> t0;
		std::auto_ptr<QuadEdge> u0;

		//make a quadilateral
		q0 = QuadEdge::makeEdge(v1, v2);
		r0 = QuadEdge::makeEdge(v4, v3);
		s0 = QuadEdge::connect(*q0, *r0);
		t0 = QuadEdge::connect(*r0, *q0);

		//printf("\n=====================\n");
		//printf("r0->orig(): %f %f\n", r0->orig().getX(), r0->orig().getY());
		//printf("r0->dest(): %f %f\n", r0->dest().getX(), r0->dest().getY());
		//printf("s0->orig(): %f %f\n", s0->orig().getX(), s0->orig().getY());
		//printf("s0->dest(): %f %f\n", s0->dest().getX(), s0->dest().getY());

		//add an interior edge to make 2 triangles
		u0 = QuadEdge::connect(*t0, *r0);
		//printf("\n=====================\n");
		//printf("q0->orig(): %f %f\n", q0->orig().getX(), q0->orig().getY());
		//printf("q0->dest(): %f %f\n", q0->dest().getX(), q0->dest().getY());
		//printf("r0->orig(): %f %f\n", r0->orig().getX(), r0->orig().getY());
		//printf("r0->dest(): %f %f\n", r0->dest().getX(), r0->dest().getY());
		//printf("s0->orig(): %f %f\n", s0->orig().getX(), s0->orig().getY());
		//printf("s0->dest(): %f %f\n", s0->dest().getX(), s0->dest().getY());
		//printf("t0->orig(): %f %f\n", t0->orig().getX(), t0->orig().getY());
		//printf("t0->dest(): %f %f\n", t0->dest().getX(), t0->dest().getY());
		//printf("u0->orig(): %f %f\n", u0->orig().getX(), u0->orig().getY());
		//printf("u0->dest(): %f %f\n", u0->dest().getX(), u0->dest().getY());
		ensure(t0->dest().equals(u0->orig()));
		ensure(u0->dest().equals(r0->orig()));

		//now swap the interior edge
		QuadEdge::swap(*u0);
		//printf("\n=====================\n");
		//printf("q0->orig(): %f %f\n", q0->orig().getX(), q0->orig().getY());
		//printf("q0->dest(): %f %f\n", q0->dest().getX(), q0->dest().getY());
		//printf("r0->orig(): %f %f\n", r0->orig().getX(), r0->orig().getY());
		//printf("r0->dest(): %f %f\n", r0->dest().getX(), r0->dest().getY());
		//printf("s0->orig(): %f %f\n", s0->orig().getX(), s0->orig().getY());
		//printf("s0->dest(): %f %f\n", s0->dest().getX(), s0->dest().getY());
		//printf("t0->orig(): %f %f\n", t0->orig().getX(), t0->orig().getY());
		//printf("t0->dest(): %f %f\n", t0->dest().getX(), t0->dest().getY());
		//printf("u0->orig(): %f %f\n", u0->orig().getX(), u0->orig().getY());
		//printf("u0->dest(): %f %f\n", u0->dest().getX(), u0->dest().getY());
		ensure(r0->dest().equals(u0->dest()));
		ensure(u0->orig().equals(q0->dest()));

		q0->free();
		r0->free();
		s0->free();
		t0->free();
		u0->free();
	}
} // namespace tut


