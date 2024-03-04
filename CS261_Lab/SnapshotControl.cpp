//---------------------------------------------------------
// file:	DeadReckoningControl.cpp
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	Calculates the position as *interpolated* from the last two known positions
//
// Copyright © 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#include "pch.h"
#include "SnapshotControl.h"


void SnapshotControl::AddSnapshot(float x, float y, float time_since_last_update_secs)
{
	time_since_last_update_secs_ = time_since_last_update_secs;
	previous_x_ = latest_x_;
	previous_y_ = latest_y_;
	latest_x_ = x;
	latest_y_ = y;
	bias_ = 0.0f;

	if (is_initialized == false)
	{
		current_x_ = previous_x_ = latest_x_;
		current_y_ = previous_x_ = latest_y_;
		is_initialized = true;
	}
}


void SnapshotControl::Update(float dt)
{
	if (is_initialized == false)
	{
		return;
	}

	// simple idea: we should move from 0 - 1 in bias in the same amount of time it took for the object to arrive
	bias_ += dt / time_since_last_update_secs_;

	// note: this will project ahead if bias is greater than 1
	current_x_ = previous_x_ + bias_ * (latest_x_ - previous_x_);
	current_y_ = previous_y_ + bias_ * (latest_y_ - previous_y_);
}


void SnapshotControl::Draw()
{
	// if we have over-run the last received value, call attention by changing the stroke to white
	if (bias_ > 1.0f)
	{
		CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
	}
	else
	{
		CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
	}

	CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255));
	CP_Graphics_DrawCircle(latest_x_, latest_y_, 20);
	CP_Graphics_DrawCircle(previous_x_, previous_y_, 20);
	CP_Graphics_DrawLine(latest_x_, latest_y_, previous_x_, previous_y_);
	CP_Graphics_DrawLine(current_x_, current_y_, previous_x_, previous_y_);
}