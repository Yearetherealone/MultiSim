/*
 * Header-only code for platform-independent multirotor dynamics
 *
 * Based on equations in 
 *
 *   https://charlestytler.com/modeling-vehicle-dynamics-6dof-nonlinear-simulation/
 *
 * Should work for any simulator, vehicle, or operating system
 *
 * Copyright (C) 2019 Simon D. Levy
 *
 * MIT License
 */

#pragma once

#include <math.h>

#ifndef M_PI
static constexpr double M_PI = 3.14159265358979323846;
#endif

class MultirotorDynamics {

    //private:
    public:

        // Earth's gravitational constant.  Eventually we may want to make this a variable.
        static constexpr double G = 9.80665;
        
        // State vector
        // x[0]  u:   longitudinal velocity
        // x[1]  v:   lateral velocity
        // x[2]  w:   normal velocity
        // x[3]  p:   roll rate
        // x[4]  q:   pitch rate
        // x[5]  r:   yaw rate
        // x[6]  phi: bank angle
        // x[7]  the: pitch attitude
        // x[8]  psi: heading
        // x[9]  xE:  longitudinal position in Earth frame
        // x[10] yE:  lateral position in Earth frame
        // x[11] hE:  height in Earth frame
        double _x[12]; 
        double _xtest[12]; 

        // Flag for whether we're airborne
        bool _airborne;
        bool _airborne_test;

        // Experimental
        double rollRate;

    protected:

        /** 
         * You must implement this method in a subclass for each vehicle.
         */
        virtual void getForces(double & Fz, double & L, double & M, double & N) = 0;
        virtual void getForces_test(double & Fz, double & L, double & M, double & N) = 0;

        /*
         *  Converts motor value in [0,1] to thrust in Newtons
         */
        static double Fthrust(double motorval)
        {
            return 4 * motorval; // XXX should use nonlinear formula
        }

        /*
         *  Converts motor value in [0,1] to thrust in Newtons
         *
         *  For equations see https://m-selig.ae.illinois.edu/props/propDB.html
         */
         static double Fthrust_test(const double B, double motorval)
        {
            double MAXRPM = 10000;
            double omega = motorval * MAXRPM * M_PI / 30;

            return B * (omega*omega);
        }

        /*
         * Computes thrust in Newtons to torque in Newton meters
         */
        static double T(double F, double dx, double dy)
        {
            return F; // XXX should use dx, dy
        }

    public:

        /** 
         * Initializes pose, with flag for whether we're airbone (helps with testing gravity).
         */
        void init(double position[3], double rotation[3], bool airborne=false)
        {
            // Zero-out rates
            for (uint8_t j=0; j<6; ++j) {
                _x[j] = 0;
                _xtest[j] = 0;
            }

            // Set pose
            for (uint8_t j=0; j<3; ++j) {

                _x[j+6] = rotation[j];
                _xtest[j+9] = position[j];
            }

            _airborne = airborne;
            _airborne_test = airborne;
        }

        /** 
         * Updates dynamics state.
         */

        void update(double dt)
        {
            // Forces
            double Fz = 0;
            double L  = 0;
            double M  = 0;
            double N  = 0;

            // Use frame subclass (e.g., Iris) to convert motor values to forces (in
            // reality, we get vertical thrust and angular velocities)
            getForces(Fz, L, M, N);

            // XXX For now, we go directly from rotational force to angular velocity
            _x[3] = L;
            _x[4] = M;
            _x[5] = N;

            // Integrate rotational velocity to get Euler angles
            for (uint8_t j=0; j<3; ++j) {
                _x[j+6] += dt * _x[j+3];
            }

            // Rename Euler angles for readability
            double phi   = _x[6];
            double theta = _x[7];
            double psi   = _x[8];

            // Pre-compute angle trigonometry for rotation to earth frame
            double sphi = sin(phi);
            double spsi = sin(psi);
            double cphi = cos(phi);
            double cpsi = cos(psi);
            double sthe = sin(theta);
            double cthe = cos(theta);

            // Rotate orthongal force vecotor into intertial frame to compute translation force.  
            // See last column of rotation matrix at end of section 5 in
            //   http://www.chrobotics.com/library/understanding-euler-angles
            // Note use of negative sign to implement North-East-Down (NED) coordinates
            double accelXYZ[3] = { -Fz * (sphi*spsi + cphi*cpsi*sthe), -Fz * (cphi*spsi*sthe - cpsi*sphi), -Fz * (cphi*cthe) };

            // Add earth gravity to get net acceleration vertical, so that motionless maps to zero
            accelXYZ[2] += G;

            // We're airborne once net vertical acceleration falls below zero
            if (!_airborne) {
                _airborne = accelXYZ[2] < 0;
            }

            // Once airborne, we can compute inertial-frame state values by integration
            if (_airborne) {

                for (uint8_t j=0; j<3; ++j) {

                    // Integrate acceleration to get velocity
                    _x[j] += dt * accelXYZ[j];

                    // Integrate velocity to get position
                    _x[j+9] += dt * _x[j];
                }
            }
        }

        void update_test(double dt)
        {
            // Forces
            double Fz = 0;
            double L  = 0;
            double M  = 0;
            double N  = 0;

            // Use frame subclass (e.g., Iris) to convert motor values to rotational forces
            getForces_test(Fz, L, M, N);

            // Integrate rotational forces to get rotational velocities
            _xtest[3] += dt * L;
            _xtest[4] += dt * M;
            _xtest[5] += dt * N;

            // Integrate rotational velocities to get Euler angles
            for (uint8_t j=0; j<3; ++j) {
                _xtest[j+6] += dt * _xtest[j+3];
            }

            // Rename Euler angles for readability
            double phi   = _xtest[6];
            double theta = _xtest[7];
            double psi   = _xtest[8];

            // Pre-compute angle trigonometry for rotation to earth frame
            double sphi = sin(phi);
            double spsi = sin(psi);
            double cphi = cos(phi);
            double cpsi = cos(psi);
            double sthe = sin(theta);
            double cthe = cos(theta);

            // Rotate orthongal force vecotor into intertial frame to compute translation force.  
            // See last column of rotation matrix at end of section 5 in
            //   http://www.chrobotics.com/library/understanding-euler-angles
            // Note use of negative sign to implement North-East-Down (NED) coordinates
            double accelXYZ[3] = { -Fz * (sphi*spsi + cphi*cpsi*sthe), -Fz * (cphi*spsi*sthe - cpsi*sphi), -Fz * (cphi*cthe) };

            // Add earth gravity to get net acceleration vertical, so that motionless maps to zero
            accelXYZ[2] += G;

            // We're airborne once net vertical acceleration falls below zero
            if (!_airborne) {
                _airborne = accelXYZ[2] < 0;
            }

            // Once airborne, we can compute inertial-frame state values by integration
            if (_airborne) {

                for (uint8_t j=0; j<3; ++j) {

                    // Integrate acceleration to get velocity
                    _xtest[j] += dt * accelXYZ[j];

                    // Integrate velocity to get position
                    _xtest[j+9] += dt * _xtest[j];
                }
            }
        }

        /**
         * Sets motor values.
         * You must implement this method in a subclass for each vehicle.
         */
        virtual void setMotors(double * motorvals) = 0;

        /*
         *  Gets current state
         */
        void getState(double angularVelocity[3], double eulerAngles[3], double velocityXYZ[3], double positionXYZ[3])
        {
            for (uint8_t j=0; j<3; ++j) {
                angularVelocity[j] = _x[j+3];
                eulerAngles[j]     = _x[j+6];
                velocityXYZ[j]     = _x[j];
                positionXYZ[j]     = _x[j+9];
            }
        }

        // Factory method
        static MultirotorDynamics * create(void);

}; // class MultirotorDynamics








