// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

#include "box2d/color.h"
#include "box2d/constants.h"
#include "box2d/id.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define B2_LITERAL(T) T
#else
#define B2_LITERAL(T) (T)
#endif

#define B2_ARRAY_COUNT(A) (int)(sizeof(A) / sizeof(A[0]))
#define B2_MAYBE_UNUSED(x) ((void)(x))
#define B2_NULL_INDEX (-1)

/// 2D vector
typedef struct b2Vec2
{
	float x, y;
} b2Vec2;

/// 2D rotation
typedef struct b2Rot
{
	/// Sine and cosine
	float s, c;
} b2Rot;

/// A 2D rigid transform
typedef struct b2Transform
{
	b2Vec2 p;
	b2Rot q;
} b2Transform;

/// A 2-by-2 Matrix
typedef struct b2Mat22
{
	/// columns
	b2Vec2 cx, cy;
} b2Mat22;

/// Axis-aligned bounding box
typedef struct b2AABB
{
	b2Vec2 lowerBound;
	b2Vec2 upperBound;
} b2AABB;

/// Ray-cast input data. The ray extends from p1 to p1 + maxFraction * (p2 - p1).
typedef struct b2RayCastInput
{
	b2Vec2 p1, p2;
	float radius;
	float maxFraction;
} b2RayCastInput;

/// Ray-cast output data. The ray hits at p1 + fraction * (p2 - p1), where p1 and p2 come from b2RayCastInput.
typedef struct b2RayCastOutput
{
	b2Vec2 normal;
	b2Vec2 point;
	float fraction;
	int32_t iterations;
	bool hit;
} b2RayCastOutput;

/// Task interface
/// This is prototype for a Box2D task. Your task system is expected to invoke the Box2D task with these arguments.
/// The task spans a range of the parallel-for: [startIndex, endIndex)
/// The thread index must correctly identify each thread in the user thread pool, expected in [0, workerCount)
/// The task context is the context pointer sent from Box2D when it is enqueued.
typedef void b2TaskCallback(int32_t startIndex, int32_t endIndex, uint32_t threadIndex, void* taskContext);

/// These functions can be provided to Box2D to invoke a task system. These are designed to work well with enkiTS.
/// Returns a pointer to the user's task object. May be nullptr.
typedef void* b2EnqueueTaskCallback(b2TaskCallback* task, int32_t itemCount, int32_t minRange, void* taskContext, void* userContext);

/// Finishes a user task object that wraps a Box2D task.
typedef void b2FinishTaskCallback(void* userTask, void* userContext);

/// Finishes all tasks.
typedef void b2FinishAllTasksCallback(void* userContext);

typedef struct b2WorldDef
{
	/// Gravity vector. Box2D has no up-vector defined.
	b2Vec2 gravity;

	/// Restitution velocity threshold, usually in m/s. Collisions above this
	/// speed have restitution applied (will bounce).
	float restitutionThreshold;

	/// Can bodies go to sleep to improve performance
	bool enableSleep;

	/// Capacity for bodies. This may not be exceeded.
	int32_t bodyCapacity;

	/// initial capacity for shapes
	int32_t shapeCapacity;

	/// Capacity for contacts. This may not be exceeded.
	int32_t contactCapacity;

	/// Capacity for joints
	int32_t jointCapacity;

	/// Stack allocator capacity. This controls how much space box2d reserves for per-frame calculations.
	/// Larger worlds require more space. b2Statistics can be used to determine a good capacity for your
	/// application.
	int32_t stackAllocatorCapacity;

	/// task system hookup
	uint32_t workerCount;
	b2EnqueueTaskCallback* enqueueTask;
	b2FinishTaskCallback* finishTask;
	b2FinishAllTasksCallback* finishAllTasks;
	void* userTaskContext;

} b2WorldDef;

/// The body type.
/// static: zero mass, zero velocity, may be manually moved
/// kinematic: zero mass, non-zero velocity set by user, moved by solver
/// dynamic: positive mass, non-zero velocity determined by forces, moved by solver
typedef enum b2BodyType
{
	b2_staticBody = 0,
	b2_kinematicBody = 1,
	b2_dynamicBody = 2,
	b2_bodyTypeCount
} b2BodyType;

/// A body definition holds all the data needed to construct a rigid body.
/// You can safely re-use body definitions. Shapes are added to a body after construction.
typedef struct b2BodyDef
{
	/// The body type: static, kinematic, or dynamic.
	/// Note: if a dynamic body would have zero mass, the mass is set to one.
	enum b2BodyType type;

	/// The world position of the body. Avoid creating bodies at the origin
	/// since this can lead to many overlapping shapes.
	b2Vec2 position;

	/// The world angle of the body in radians.
	float angle;

	/// The linear velocity of the body's origin in world co-ordinates.
	b2Vec2 linearVelocity;

	/// The angular velocity of the body.
	float angularVelocity;

	/// Linear damping is use to reduce the linear velocity. The damping parameter
	/// can be larger than 1.0f but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	float linearDamping;

	/// Angular damping is use to reduce the angular velocity. The damping parameter
	/// can be larger than 1.0f but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	float angularDamping;

	/// Scale the gravity applied to this body.
	float gravityScale;

	/// Use this to store application specific body data.
	void* userData;

	/// Set this flag to false if this body should never fall asleep. Note that
	/// this increases CPU usage.
	bool enableSleep;

	/// Is this body initially awake or sleeping?
	bool isAwake;

	/// Should this body be prevented from rotating? Useful for characters.
	bool fixedRotation;

	/// Does this body start out enabled?
	bool isEnabled;
} b2BodyDef;

/// This holds contact filtering data.
typedef struct b2Filter
{
	/// The collision category bits. Normally you would just set one bit.
	uint32_t categoryBits;

	/// The collision mask bits. This states the categories that this
	/// shape would accept for collision.
	uint32_t maskBits;

	/// Collision groups allow a certain group of objects to never collide (negative)
	/// or always collide (positive). Zero means no collision group. Non-zero group
	/// filtering always wins against the mask bits.
	int32_t groupIndex;
} b2Filter;

/// Used to create a shape
typedef struct b2ShapeDef
{
	/// Use this to store application specific shape data.
	void* userData;

	/// The friction coefficient, usually in the range [0,1].
	float friction;

	/// The restitution (elasticity) usually in the range [0,1].
	float restitution;

	/// The density, usually in kg/m^2.
	float density;

	/// Contact filtering data.
	b2Filter filter;

	/// A sensor shape collects contact information but never generates a collision
	/// response.
	bool isSensor;

} b2ShapeDef;

static const b2Filter b2_defaultFilter = {0x00000001, 0xFFFFFFFF, 0};

/// Make a world definition with default values.
static inline b2WorldDef b2DefaultWorldDef(void)
{
	b2WorldDef def = {0};
	def.gravity = B2_LITERAL(b2Vec2){0.0f, -10.0f};
	def.restitutionThreshold = 1.0f * b2_lengthUnitsPerMeter;
	def.enableSleep = true;
	def.bodyCapacity = 8;
	def.shapeCapacity = 8;
	def.contactCapacity = 8;
	def.jointCapacity = 8;
	def.stackAllocatorCapacity = 1024 * 1024;
	return def;
}

/// Make a body definition with default values.
static inline b2BodyDef b2DefaultBodyDef(void)
{
	b2BodyDef def = {0};
	def.type = b2_staticBody;
	def.position = B2_LITERAL(b2Vec2){0.0f, 0.0f};
	def.angle = 0.0f;
	def.linearVelocity = B2_LITERAL(b2Vec2){0.0f, 0.0f};
	def.angularVelocity = 0.0f;
	def.linearDamping = 0.0f;
	def.angularDamping = 0.0f;
	def.gravityScale = 1.0f;
	def.userData = NULL;
	def.enableSleep = true;
	def.isAwake = true;
	def.fixedRotation = false;
	def.isEnabled = true;
	return def;
}

static inline struct b2ShapeDef b2DefaultShapeDef(void)
{
	b2ShapeDef def = {0};
	def.friction = 0.6f;
	def.restitution = 0.0f;
	def.density = 0.0f;
	def.filter = b2_defaultFilter;
	def.isSensor = false;
	return def;
}
