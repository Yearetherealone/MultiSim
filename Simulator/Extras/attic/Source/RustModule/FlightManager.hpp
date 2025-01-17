#define WIN32_LEAN_AND_MEAN

#include "../MainModule/FlightManager.hpp"
#include "../MainModule/Dynamics.hpp"
#include "../MainModule/Joystick.h"

class FRustFlightManager : public FFlightManager {

    private:

        typedef struct {

            float error_integral;
            bool  in_band;
            float target;

        } alt_hold_t;

        typedef struct {

            float throttle;
            float roll;
            float pitch;
            float yaw;

        } demands_t;

        typedef struct {

            float m1;
            float m2;
            float m3;
            float m4;

        } motors_t;

        typedef struct {

            demands_t demands;
            Dynamics::vehicle_state_t vehicle_state;
            alt_hold_t alt_hold_pid;
            motors_t motors;

        } hackflight_t;

        typedef hackflight_t (*hackflight_fun_t) (hackflight_t * hackflight);

        hackflight_fun_t _run_hackflight;

        Dynamics * _dynamics;

        IJoystick * _joystick;

        static float scaleAxis(float value);

    protected:

        virtual void getMotors(double time, double* values) override;

    public:

        FRustFlightManager(APawn * pawn, Dynamics * dynamics);

        ~FRustFlightManager();

        void tick(void);

}; // FRustFlightManager
