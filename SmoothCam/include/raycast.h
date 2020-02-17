#pragma once
#include "pch.h"

namespace Raycast {
	// These require more inspection - They all contain vtables of assorted physics shapes
	// It would be nice to figure these out to the point that a TESObjectREFR could be
	// extracted from them
	struct hkpGenericShapeData {
		intptr_t* unk;
		uint32_t shapeType;
	};

	struct rayHitShapeInfo {
		hkpGenericShapeData* hitShape;
		uint32_t unk;
	};

	typedef __declspec(align(16)) struct bhkRayResult {
		// The actual param given to the engine
		union {
			uint32_t data[16];
			struct {
				// Might be surface normal
				glm::vec4 rayUnkPos;
				// The normal vector of the ray
				glm::vec4 rayNormal;

				rayHitShapeInfo colA;
				rayHitShapeInfo colB;
			};
		};

		// Custom utility data, not part of the game
		
		// True if the trace hit something before reaching it's end position
		bool hit;
		// If the ray hit a character actor, this will point to it
		Character* hitCharacter;
		// The length of the ray from start to hitPos
		float rayLength;
		// The position the ray hit, in world space
		glm::vec4 hitPos;

		// pad to 128
		uint64_t _pad[3];

		bhkRayResult() noexcept : hit(false), hitCharacter(nullptr) {}
	} RayResult;
	static_assert(sizeof(RayResult) == 128);

	// Cast a ray from 'start' to 'end', returning the first thing it hits
	//	Params:
	//		glm::vec4 start:     Starting position for the trace in world space
	//		glm::vec4 end:       End position for the trace in world space
	//		float traceHullSize: Size of the collision hull used for the trace
	//
	// Returns:
	//	RayResult:
	//		A structure holding the results of the ray cast.
	//		If the ray hit something, result.hit will be true.
	RayResult CastRay(glm::vec4 start, glm::vec4 end, float traceHullSize);
}