/*
 Helper class for pawns using DJI Phantom frame
 *
 * Copyright (C) 2019 Simon D. Levy
 *
 * MIT License
 */

#pragma once

#include "../Multirotor.hpp"

#include "../MainModule/dynamics/QuadXAP.hpp"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

// Structures to hold static mesh initializations
DECLARE_STATIC_MESH(FFrameStatics, "Phantom/Frame.Frame", FrameStatics)
DECLARE_STATIC_MESH(FPropCWStatics, "Phantom/PropCW.PropCW", PropCWStatics)
DECLARE_STATIC_MESH(FPropCCWStatics, "Phantom/PropCCW.PropCCW", PropCCWStatics)

class Phantom {

    private:

        Dynamics::vehicle_params_t vparams = {

            // Estimated
            5.E-06, // b force constatnt [F=b*w^2]
            2.E-06, // d torque constant [T=d*w^2]

            // https://www.dji.com/phantom-4/info
            1.380,  // m mass [kg]

            // Estimated
            2,      // Ix [kg*m^2] 
            2,      // Iy [kg*m^2] 
            3,      // Iz [kg*m^2] 
            38E-04, // Jr prop inertial [kg*m^2] 

            0.350,  // l arm length [m]

            15000 // maxrpm
        };

    public:

        QuadXAPDynamics dynamics = QuadXAPDynamics(vparams);

        MultirotorVehicle vehicle = MultirotorVehicle(&dynamics, 4);

    private:

        // Threaded worker for flight control
        FFlightManager * _flightManager = NULL;

    public:

        void build(APawn * pawn)
        {
            vehicle.buildFull(pawn, FrameStatics.mesh.Get());

            // Add propellers
            addRotor(PropCCWStatics.mesh.Get(), +1, +1);
            addRotor(PropCCWStatics.mesh.Get(), -1, -1);
            addRotor(PropCWStatics.mesh.Get(), +1, -1);
            addRotor(PropCWStatics.mesh.Get(), -1, +1);

            _flightManager = NULL;
        }

        void PostInitializeComponents()
        {
            vehicle.PostInitializeComponents();
        }

        void BeginPlay(FFlightManager * flightManager)
        {
            _flightManager = flightManager;

            vehicle.BeginPlay(flightManager);
        }

        void EndPlay(void)
        {
            FThreadedManager::stopThread((FThreadedManager **)&_flightManager);
        }

        void Tick(float DeltaSeconds)
        {
            vehicle.Tick(DeltaSeconds);
        }

        void addCamera(Camera * camera)
        {
            vehicle.addCamera(camera);
        }

        void addRotor(UStaticMesh * mesh, int8_t dx, int8_t dy)
        {
            vehicle.addRotor(mesh, dx*0.12, dy*0.12, 0.17);
        }

}; // class Phantom 
