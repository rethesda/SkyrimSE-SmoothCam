#include "camera_states/thirdperson/thirdperson_combat.h"
#include "thirdperson.h"

Camera::State::ThirdpersonCombatState::ThirdpersonCombatState(Thirdperson* camera) noexcept : BaseThird(camera) {

}

void Camera::State::ThirdpersonCombatState::OnBegin(const PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera,
	BaseThird* fromState) noexcept
{

}

void Camera::State::ThirdpersonCombatState::OnEnd(const PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera,
	BaseThird* nextState) noexcept
{
	StateHandOff(nextState);
}

void Camera::State::ThirdpersonCombatState::Update(PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera)
	noexcept
{
	// Get our computed local-space xyz offset.
	const auto cameraLocal = GetCameraOffsetStatePosition();
	// Get the base world position for the camera which we will offset with the local-space values.
	const auto worldTarget = GetCameraWorldPosition(cameraRef, camera);
	// Transform the camera offsets based on the computed view matrix
	const auto transformedLocalPos = GetTransformedCameraLocalPosition();

	glm::vec3 preFinalPos;
	if (GetConfig()->separateLocalInterp) {
		// Handle separate local-space interpolation
		const auto loc = UpdateInterpolatedLocalPosition(transformedLocalPos);

		const auto& last = GetLastCameraPosition();
		// Last offset position from ref
		const auto lastWorld = last.world - last.local;

		// And the world target
		const auto lerpedWorldPos = UpdateInterpolatedWorldPosition(
			player, lastWorld, worldTarget,
			glm::length(lastWorld - worldTarget)
		);

		// Compute offset clamping if enabled
		preFinalPos = ComputeOffsetClamping(cameraRef, worldTarget, lerpedWorldPos) + loc;

	} else {
		// Combined case

		// Add the final local space transformation to the player postion
		const auto targetWorldPos = worldTarget + transformedLocalPos;

		// Now lerp it based on camera distance to player position
		const auto lerpedWorldPos = UpdateInterpolatedWorldPosition(
			player, GetLastCameraPosition().world, targetWorldPos,
			glm::length(targetWorldPos - worldTarget)
		);

		// Compute offset clamping if enabled
		preFinalPos = ComputeOffsetClamping(cameraRef, transformedLocalPos, worldTarget, lerpedWorldPos);
	}

	glm::vec3 f, s, u, coef;
	mmath::DecomposeToBasis(
		preFinalPos,
		{ GetCameraRotation().euler.x, 0.0f, GetCameraRotation().euler.y },
		f, s, u, coef
	);

	// Cast from the player out towards the X offset, rotated
	// The end position of that ray becomes the origin for our primary distance ray
	// for camera collision.
	// Doing this ensures the origin of our collision ray is never occluded - We just
	// need to make sure the hull size for our origin test ray is larger than the primary
	// collision ray.
	constexpr auto hullSize = 30.0f;

	// Ray origin from the player, plus Z offset
	const auto playerOrigin = worldTarget + glm::vec3(0.0f, 0.0f, cameraLocal.z);
	// Towards the +X offset
	auto rayXOrigin = playerOrigin + (f * cameraLocal.x);
	const auto resultPtoX = Raycast::CastRay(glm::vec4(playerOrigin, 0.0f), glm::vec4(rayXOrigin, 0.0f), hullSize);

	// And if we hit, that hit position becomes the new origin for the next ray
	if (resultPtoX.hit)
		rayXOrigin = resultPtoX.hitPos + (resultPtoX.rayNormal * glm::min(resultPtoX.rayLength, hullSize));

	// Otherwise, we just cast from +X offset like normal as nothing is overlapping
	// Cast from origin towards the camera, Get back final position
	const auto finalPos = ComputeRaycast(rayXOrigin, preFinalPos);

	// Set the position to the ray hit position
	// This sets the rendering position of the camera, and applies game offsets
	SetCameraPosition(finalPos, player, camera);
	// And store our world position BEFORE collision
	GetCameraPosition().SetWorldPosition(preFinalPos);
	// Update crosshair visibility
	UpdateCrosshair(player, camera);
}