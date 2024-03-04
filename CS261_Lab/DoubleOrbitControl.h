//---------------------------------------------------------
// file:	DoubleOrbitControl.h
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	Calculates a moving position within a double-orbital (figure-eight).
//
// remarks: This motion represents "unpredictable" player control, in our scenarios.
//
// Copyright © 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#pragma once
#include "PlayerControl.h"


/// <summary>
/// Calculates a moving position within a double-orbital (figure-eight).
/// </summary>
/// <remarks>This motion represents "unpredictable" player control, in our scenarios.</remarks>
class DoubleOrbitControl
	: public PlayerControl
{
public:
	DoubleOrbitControl(float left_center_x, float left_center_y, float radius, float duration_secs);

	virtual void Update(float dt) override;
	virtual void Draw() override { }

	inline float GetCurrentVelocityX() const { return current_velocity_x_; }
	inline float GetCurrentVelocityY() const { return current_velocity_y_; }

private:
	void CalculateCurrentPosition();

	float current_velocity_x_ = 0.0f, current_velocity_y_ = 0.0f;
	
	float left_center_x_, left_center_y_;
	float right_center_x_, right_center_y_; 
	float radius_;
	float duration_secs_;
	bool isOrbitingLeft_;
	float current_angle_;
};

