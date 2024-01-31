#pragma once

#include <windows.h>
#include <d3d11.h>
#include <directxmath.h>
#include <cstdint>

#include "Mesh.h"


using namespace DirectX;

using Vec = XMVECTOR;

struct Cube : public Mesh {


public:


	static Mesh getCubeMesh() 
	
	{
		float dx = 1, dy = 1, dz = 1;

		XMVECTOR n0{ 0.0f, 0.0f, -1.0f }; // devant
		XMVECTOR n1{ 0.0f, 0.0f, 1.0f }; // arrière
		XMVECTOR n2{ 0.0f, -1.0f, 0.0f }; // dessous
		XMVECTOR n3{ 0.0f, 1.0f, 0.0f }; // dessus
		XMVECTOR n4{ -1.0f, 0.0f, 0.0f }; // face gauche
		XMVECTOR n5{ 1.0f, 0.0f, 0.0f }; // face droite

		XMVECTOR point[8] =
		{
			{-dx / 2, dy / 2, -dz / 2	},
			{dx / 2, dy / 2, -dz / 2	},
			{dx / 2, -dy / 2, -dz / 2	},
			{-dx / 2, -dy / 2, -dz / 2	},
			{-dx / 2, dy / 2, dz / 2	},
			{-dx / 2, -dy / 2, dz / 2	},
			{dx / 2, -dy / 2, dz / 2	},
			{dx / 2, dy / 2, dz / 2		}
		};

		std::vector<Vertex> vertices =
		{

			// Front
			{point[0], n0, Vec{0.0f, 0.0f}},	{point[1], n0, Vec{1.0f, 0.0f}},
			{point[2], n0, Vec{1.0f, 1.0f}},	{point[3], n0, Vec{0.0f, 1.0f}},
			

			//Back
			{point[4], n1, Vec{0.0f, 1.0f}},	{point[5], n1, Vec{0.0f, 0.0f}},
			{point[6], n1, Vec{1.0f, 0.0f}},	{point[7], n1, Vec{1.0f, 1.0f}},
			

			// down
			{point[3], n2, Vec{0.0f, 0.0f}},	{point[2], n2, Vec{1.0f, 0.0f}},
			{point[6], n2, Vec{1.0f, 1.0f}},	{point[5], n2, Vec{0.0f, 1.0f}},


			// up
			{point[0], n3, Vec{0.0f, 1.0f}},	{point[4], n3, Vec{0.0f, 0.0f}},
			{point[7], n3, Vec{1.0f, 0.0f}},	{point[1], n3, Vec{1.0f, 1.0f}},
			

			// left
			{point[0], n4, Vec{0.0f, 0.0f}},	{point[3], n4, Vec{1.0f, 0.0f}},
			{point[5], n4, Vec{1.0f, 1.0f}},	{point[4], n4, Vec{0.0f, 1.0f}},
			

			// right
			{point[1], n5, Vec{0.0f, 0.0f}},	{point[7], n5, Vec{1.0f, 0.0f}},
			{point[6], n5, Vec{1.0f, 1.0f}},	{point[2], n5, Vec{0.0f, 1.0f}},
		};
		std::vector<IndexBuffer::size_type> indices
		{
			1,0,2,		// devant 
			2,0,3,		// devant 
			6,5,7,		// arrière 
			7,5,4,		// arrière
			9,8,10,		// dessous
			10,8,11,	// dessous 
			14,13,15,	// dessus 
			15,13,12,	// dessus 
			16,19,17,	// gauche 
			17,19,18,	// gauche 
			21,20,22,	// droite 
			22,20,23	// droite 
		};


	

		return Mesh(vertices, indices);

	}

	static Mesh getInvertedCubeMesh()

	{
		float dx = 1, dy = 1, dz = 1;

		XMVECTOR n0{ 0.0f, 0.0f, -1.0f };	// devant
		XMVECTOR n1{ 0.0f, 0.0f, 1.0f };	// arrière
		XMVECTOR n2{ 0.0f, -1.0f, 0.0f };	// dessous
		XMVECTOR n3{ 0.0f, 1.0f, 0.0f };	// dessus
		XMVECTOR n4{ -1.0f, 0.0f, 0.0f };	// face gauche
		XMVECTOR n5{ 1.0f, 0.0f, 0.0f };	// face droite


		/*
		  4           7
		   +--------+
		  /        /|
		 /        / |
	0	+--------+ 1|
		|        |  |
		|  5     |  + 6
		|        | /
		|        |/
	 3  +--------+ 2
		
		*/


		XMVECTOR point[8] =
		{
			{-dx / 2, dy / 2, -dz / 2	}, // 0
			{dx / 2, dy / 2, -dz / 2	}, // 1
			{dx / 2, -dy / 2, -dz / 2	}, // 2
			{-dx / 2, -dy / 2, -dz / 2	}, // 3
			{-dx / 2, dy / 2, dz / 2	}, // 4
			{-dx / 2, -dy / 2, dz / 2	}, // 5
			{dx / 2, -dy / 2, dz / 2	}, // 6
			{dx / 2, dy / 2, dz / 2		}  // 7
		};

		std::vector<Vertex> vertices =
		{

			// Front
			{point[0], n0, Vec{0.0f, 0.0f}},	{point[1], n0, Vec{1.0f, 0.0f}},
			{point[2], n0, Vec{1.0f, 1.0f}},	{point[3], n0, Vec{0.0f, 1.0f}},


			//Back
			{point[4], n1, Vec{0.0f, 1.0f}},	{point[5], n1, Vec{0.0f, 0.0f}},
			{point[6], n1, Vec{1.0f, 0.0f}},	{point[7], n1, Vec{1.0f, 1.0f}},


			// down
			{point[3], n2, Vec{0.0f, 0.0f}},	{point[2], n2, Vec{1.0f, 0.0f}},
			{point[6], n2, Vec{1.0f, 1.0f}},	{point[5], n2, Vec{0.0f, 1.0f}},


			// up
			{point[0], n3, Vec{0.0f, 1.0f}},	{point[4], n3, Vec{0.0f, 0.0f}},
			{point[7], n3, Vec{1.0f, 0.0f}},	{point[1], n3, Vec{1.0f, 1.0f}},


			// left
			{point[0], n4, Vec{0.0f, 0.0f}},	{point[3], n4, Vec{1.0f, 0.0f}},
			{point[5], n4, Vec{1.0f, 1.0f}},	{point[4], n4, Vec{0.0f, 1.0f}},


			// right
			{point[1], n5, Vec{0.0f, 0.0f}},	{point[7], n5, Vec{1.0f, 0.0f}},
			{point[6], n5, Vec{1.0f, 1.0f}},	{point[2], n5, Vec{0.0f, 1.0f}},
		};
		std::vector<IndexBuffer::size_type> indices
		{
			1,2		,0,		// devant 
			2,3		,0,		// devant 
			
			6,7		,5,		// arrière 
			7,4		,5,		// arrière
			
			9,10	,8,		// dessous
			10,11	,8,		// dessous 
			
			14,15	,13,	// dessus 
			15,12	,13,	// dessus 
			
			16,17	,19,	// gauche 
			17,18	,19,	// gauche 

			21,22	,20,	// droite 
			22,23	,20		// droite 
		};

		return Mesh(vertices, indices);

	}

};