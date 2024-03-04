//---------------------------------------------------------
// file:	DeadReckoningControl.h
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	Calculates the position as *interpolated* from the last two known positions
//
// Copyright © 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#pragma once
#include "pch.h"
#include <deque>
#include "PlayerControl.h"


/// <summary>
/// Calculates the position as *interpolated* from the last two known positions.
/// </summary>
class SnapshotControl
	: public PlayerControl
{
public:
	void AddSnapshot(float x, float y, float time_since_last_update_secs);

	void Update(float dt);
	void Draw();

private:
	float latest_x_ = 0.0f, latest_y_ = 0.0f;
	float previous_x_ = 0.0f, previous_y_ = 0.0f;
	float time_since_last_update_secs_ = 1.0f;
	float bias_ = 0.0f;
	bool is_initialized = false;
};