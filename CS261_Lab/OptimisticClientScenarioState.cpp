//---------------------------------------------------------
// file:	OptimisticClientScenarioState.cpp
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	The optimistic scenario for the client, in which the remote is the authority.
//
// Copyright © 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#include "pch.h"
#include "OptimisticClientScenarioState.h"
#include "PacketSerializer.h"

const int kNetworkBufferSize = 1024;
const float kTimeBetweenClientSend_Secs = 0.1f; // acceptable latency on client control updates: 100ms (plus wire, etc.)


OptimisticClientScenarioState::OptimisticClientScenarioState(const SOCKET socket)
	: NetworkedScenarioState(socket, false),
	active_control_(OptimisticClientScenarioState::Active_Control::Simple),
	is_drawing_controls_(false),
	local_frame_(0),
	remote_frame_(0),
	send_timer_secs_(0.0f), // always start with a packet
	time_since_last_recv_(0.0f),
	packet_(kNetworkBufferSize)
{
	local_player_.color = CP_Color_Create(255, 0, 0, 255);
	remote_player_.color = CP_Color_Create(0, 0, 255, 255);
}


OptimisticClientScenarioState::~OptimisticClientScenarioState() = default;


void OptimisticClientScenarioState::Update()
{
	NetworkedScenarioState::Update();

	if (CP_Input_KeyTriggered(CP_KEY::KEY_A))
	{
		switch (active_control_)
		{
		case Active_Control::Simple:
			active_control_ = Active_Control::Dead_Reckoning;
			break;
		case Active_Control::Dead_Reckoning:
			active_control_ = Active_Control::Snapshot;
			break;
		case Active_Control::Snapshot:
			active_control_ = Active_Control::Simple;
			break;
		}
	}
	if (CP_Input_KeyTriggered(CP_KEY::KEY_D))
	{
		is_drawing_controls_ = !is_drawing_controls_;
	}

	float system_dt = CP_System_GetDt();
	const bool is_local_paused = CP_Input_KeyDown(KEY_SPACE);

	simple_local_control_.Update(system_dt);
	simple_remote_control_.Update(system_dt);
	snapshot_local_control_.Update(system_dt);
	snapshot_remote_control_.Update(system_dt);
	dr_local_control_.Update(system_dt);
	dr_remote_control_.Update(system_dt);

	switch (active_control_)
	{
	case Active_Control::Simple:
		local_player_.SetPosition(simple_local_control_.GetCurrentX(), simple_local_control_.GetCurrentY());
		remote_player_.SetPosition(simple_remote_control_.GetCurrentX(), simple_remote_control_.GetCurrentY());
		break;
	case Active_Control::Dead_Reckoning:
		local_player_.SetPosition(dr_local_control_.GetCurrentX(), dr_local_control_.GetCurrentY());
		remote_player_.SetPosition(dr_remote_control_.GetCurrentX(), dr_remote_control_.GetCurrentY());
		break;
	case Active_Control::Snapshot:
		local_player_.SetPosition(snapshot_local_control_.GetCurrentX(), snapshot_local_control_.GetCurrentY());
		remote_player_.SetPosition(snapshot_remote_control_.GetCurrentX(), snapshot_remote_control_.GetCurrentY());
		break;
	}

	time_since_last_recv_ += system_dt;
	send_timer_secs_ -= system_dt;

	if (send_timer_secs_ < 0.0f)
	{
		packet_.Reset();
		PacketSerializer::WriteValue<u_long>(packet_, ++local_frame_);
		PacketSerializer::WriteValue<bool>(packet_, is_local_paused);
		send(socket_, packet_.GetRoot(), packet_.GetUsedSpace(), 0);
		send_timer_secs_ = kTimeBetweenClientSend_Secs;
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
			float host_x, host_y, host_velocity_x, host_velocity_y;
			float non_host_x, non_host_y, non_host_velocity_x, non_host_velocity_y;
			// CONVENTION: host writes its own values first
			PacketSerializer::ReadValue<float>(packet_, host_x);
			PacketSerializer::ReadValue<float>(packet_, host_y);
			PacketSerializer::ReadValue<float>(packet_, host_velocity_x);
			PacketSerializer::ReadValue<float>(packet_, host_velocity_y);
			PacketSerializer::ReadValue<float>(packet_, non_host_x);
			PacketSerializer::ReadValue<float>(packet_, non_host_y);
			PacketSerializer::ReadValue<float>(packet_, non_host_velocity_x);
			PacketSerializer::ReadValue<float>(packet_, non_host_velocity_y);
			// store the data in all of the controls
			simple_local_control_.SetLastKnown(non_host_x, non_host_y);
			simple_remote_control_.SetLastKnown(host_x, host_y);
			dr_local_control_.SetLastKnown(non_host_x, non_host_y, non_host_velocity_x, non_host_velocity_y, time_since_last_recv_);
			dr_remote_control_.SetLastKnown(host_x, host_y, host_velocity_x, host_velocity_y, time_since_last_recv_);
			snapshot_local_control_.AddSnapshot(non_host_x, non_host_y, time_since_last_recv_);
			snapshot_remote_control_.AddSnapshot(host_x, host_y, time_since_last_recv_);
		}
		time_since_last_recv_ = 0.0f;
	}
}


void OptimisticClientScenarioState::Draw()
{
	ScenarioState::Draw();

	local_player_.Draw();
	remote_player_.Draw();

	// draw the debug vizualizations of our replication controls
	if (is_drawing_controls_)
	{
		switch (active_control_)
		{
		case Active_Control::Dead_Reckoning:
			dr_local_control_.Draw();
			dr_remote_control_.Draw();
			break;
		case Active_Control::Snapshot:
			snapshot_local_control_.Draw();
			snapshot_remote_control_.Draw();
			break;
		}
	}
}


std::string OptimisticClientScenarioState::GetDescription() const
{
	std::string description("Optimistic Scenario, Client, Local: ");
	description += std::to_string(local_frame_);
	description += ", Remote: ";
	description += std::to_string(remote_frame_);
	switch (active_control_)
	{
	case Active_Control::Simple:
		description += ", Simple";
		break;
	case Active_Control::Dead_Reckoning:
		description += ", Dead Reckoning";
		break;
	case Active_Control::Snapshot:
		description += ", Snapshot";
		break;
	}
	if (is_drawing_controls_)
	{
		description += ", Drawing";
	}
	return description;
}


std::string OptimisticClientScenarioState::GetInstructions() const
{
	return "Hold SPACE to halt the local (red) player, A to toggle controls, D to toggle drawing";
}


bool OptimisticClientScenarioState::HandleSocketError(const char* error_text)
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