//---------------------------------------------------------
// file:	DeadReckoningControl.h
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	Calculates the position as *predicted* from the last known position and velocity.
//
// Copyright © 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#pragma once
#include "pch.h"
#include "PlayerControl.h"


/// <summary>
/// Calculates the position as *predicted* from the last known position and velocity.
/// </summary>
class DeadReckoningControl
	: public PlayerControl
{
public:
	void SetLastKnown(float position_x, float position_y, float velocity_x, float velocity_y, float time_since_last_update_secs);

	virtual void Update(float dt) override;
	virtual void Draw() override;

private:
	float current_velocity_x_ = 0.0f, current_velocity_y_ = 0.0f;
	float current_acceleration_x_ = 0.0f, current_acceleration_y_ = 0.0f;
	bool is_initialized = false;

	// retaining the last known position and velocity for visualization
	float last_known_position_x_ = 0.0f, last_known_position_y_ = 0.0f;
	float last_known_velocity_x_ = 0.0f, last_known_velocity_y_ = 0.0f;
};