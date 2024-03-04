//---------------------------------------------------------
// file:	SimpleSyncControl.cpp
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	Simply stores the last-known position and uses that.
//
// Copyright © 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#include "pch.h"
#include "SimpleSyncControl.h"


void SimpleSyncControl::SetLastKnown(float position_x, float position_y)
{
	current_x_ = position_x;
	current_y_ = position_y;
}