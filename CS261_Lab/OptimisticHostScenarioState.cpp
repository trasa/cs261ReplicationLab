//---------------------------------------------------------
// file:	OptimisticHostScenarioState.cpp
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	The optimistic scenario for the host, in which the local system is the authority.
//
// Copyright © 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#include "pch.h"
#include "OptimisticHostScenarioState.h"
#include "PacketSerializer.h"
#include "DoubleOrbitControl.h"

const int kNetworkBufferSize = 1024;


OptimisticHostScenarioState::OptimisticHostScenarioState(const SOCKET socket)
	: NetworkedScenarioState(socket, true),
	local_control_(200.0f, 250.0f, 100.0f, 1.0f),
	remote_control_(200.0f, 150.0f, 100.0f, 2.0f),
	is_remote_paused_(false),
	local_frame_(0),
	remote_frame_(0),
	send_timer_secs_(0.0f), // always start with a packet
	target_time_between_send_(0.0f),
	packet_(kNetworkBufferSize)
{
	local_player_.color = CP_Color_Create(255, 0, 0, 255);
	remote_player_.color = CP_Color_Create(0, 0, 255, 255);
}


OptimisticHostScenarioState::~OptimisticHostScenarioState() = default;


void OptimisticHostScenarioState::Update()
{
	NetworkedScenarioState::Update();

	if (CP_Input_KeyTriggered(CP_KEY::KEY_W))
	{
		target_time_between_send_ += 0.1f;
		if (target_time_between_send_ > 0.5f)
		{
			target_time_between_send_ = 0.0f;
		}
	}

	float system_dt = CP_System_GetDt();
	const bool is_local_paused = CP_Input_KeyDown(KEY_SPACE);
	// always send a packet when the server pauses...
	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		send_timer_secs_ = 0.0f; 
	}

	local_control_.Update(!is_local_paused ? system_dt : 0.0f);
	remote_control_.Update(!is_remote_paused_ ? system_dt : 0.0f);

	local_player_.SetPosition(local_control_.GetCurrentX(), local_control_.GetCurrentY());
	remote_player_.SetPosition(remote_control_.GetCurrentX(), remote_control_.GetCurrentY());

	send_timer_secs_ -= system_dt;

	if (send_timer_secs_ < 0.0f)
	{
		packet_.Reset();
		PacketSerializer::WriteValue<u_long>(packet_, ++local_frame_);
		// CONVENTION: host writes its own values first
		PacketSerializer::WriteValue<float>(packet_, local_control_.GetCurrentX());
		PacketSerializer::WriteValue<float>(packet_, local_control_.GetCurrentY());
		PacketSerializer::WriteValue<float>(packet_, local_control_.GetCurrentVelocityX());
		PacketSerializer::WriteValue<float>(packet_, local_control_.GetCurrentVelocityY());
		PacketSerializer::WriteValue<float>(packet_, remote_control_.GetCurrentX());
		PacketSerializer::WriteValue<float>(packet_, remote_control_.GetCurrentY());
		PacketSerializer::WriteValue<float>(packet_, remote_control_.GetCurrentVelocityX());
		PacketSerializer::WriteValue<float>(packet_, remote_control_.GetCurrentVelocityY());
		send(socket_, packet_.GetRoot(), packet_.GetUsedSpace(), 0);
		send_timer_secs_ = target_time_between_send_;
	}

	packet_.Reset();
	const auto res = recv(socket_, packet_.GetRoot(), packet_.GetRemainingSpace(), 0);
	if (res > 0)
	{
		u_long received_frame;
		PacketSerializer::ReadValue<u_long>(packet_, received_frame);
		// only use data if it's newer than the last frame we received
		if (received_frame > remote_frame_)
		{
			remote_frame_ = received_frame;
			// the host only receives control updates, while the client receives all positions
			PacketSerializer::ReadValue<bool>(packet_, is_remote_paused_);
		}
	}
}


void OptimisticHostScenarioState::Draw()
{
	ScenarioState::Draw();

	local_player_.Draw();
	remote_player_.Draw();

	local_control_.Draw();
	remote_control_.Draw();
}


std::string OptimisticHostScenarioState::GetDescription() const
{
	std::string description("Optimistic Scenario, Host, Local: ");
	description += std::to_string(local_frame_);
	description += ", Remote: ";
	description += std::to_string(remote_frame_);
	description += ", Send Target: ";
	description += std::to_string((int)(target_time_between_send_ * 1000));
	description += "ms";
	return description;
}


std::string OptimisticHostScenarioState::GetInstructions() const
{
	return "Hold SPACE to halt the local (red) player, W to increase Send Target";
}


bool OptimisticHostScenarioState::HandleSocketError(const char* error_text)
{
	const auto wsa_error = WSAGetLastError();

	// ignore WSAEWOULDBLOCK
	if (wsa_error == WSAEWOULDBLOCK)
	{
		return false;
	}

	// log unexpected errors and return to the default game mode
	std::cerr << "Optimistic WinSock Error: " << error_text << wsa_error << std::endl;

	// close the socket and clear it
	// -- this should trigger a GameStateManager reset in the next Update
	closesocket(socket_);
	socket_ = INVALID_SOCKET;

	return true;
}