//---------------------------------------------------------
// file:	SimpleSyncControl.h
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	Simply stores the last-known position and uses that.
//
// Copyright © 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#pragma once
#include "pch.h"
#include "PlayerControl.h"


/// <summary>
/// Simply stores the last-known position and uses that.
/// </summary>
class SimpleSyncControl
	: public PlayerControl
{
public:
	void SetLastKnown(float position_x, float position_y);

	virtual void Update(float dt) override { }
	virtual void Draw() override { };
};