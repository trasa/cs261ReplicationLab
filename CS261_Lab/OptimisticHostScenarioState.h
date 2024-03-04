//---------------------------------------------------------
// file:	OptimisticHostScenarioState.h
// author:	Matthew Picioccio
// email:	matthew.picioccio@digipen.edu
//
// brief:	The optimistic scenario for the host, in which the local system is the authority.
//
// Copyright � 2021 DigiPen, All rights reserved.
//---------------------------------------------------------
#pragma once
#include "NetworkedScenarioState.h"
#include "Player.h"
#include "Packet.h"
#include "DoubleOrbitControl.h"


/// <summary>
/// The optimistic scenario for the host, in which the local system is the authority.
/// </summary>
class OptimisticHostScenarioState final :
    public NetworkedScenarioState
{
public:
    OptimisticHostScenarioState(const SOCKET socket);
    ~OptimisticHostScenarioState() override;

    OptimisticHostScenarioState(const OptimisticHostScenarioState&) = delete;
    OptimisticHostScenarioState(OptimisticHostScenarioState&&) = delete;
    OptimisticHostScenarioState& operator=(const OptimisticHostScenarioState&) = delete;
    OptimisticHostScenarioState& operator=(OptimisticHostScenarioState&&) = delete;

    void Update() override;
    void Draw() override;

    std::string GetDescription() const override;
    std::string GetInstructions() const override;

private:
    bool HandleSocketError(const char* error_text);

    DoubleOrbitControl local_control_;
    DoubleOrbitControl remote_control_;

    Player local_player_;
    Player remote_player_;

    bool is_remote_paused_;

    u_long local_frame_;
    u_long remote_frame_;
    float send_timer_secs_;
    float target_time_between_send_;

    Packet packet_;
};