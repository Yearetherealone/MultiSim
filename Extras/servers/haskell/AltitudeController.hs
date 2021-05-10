{--
  PID Controller

  Copyright(C) 2021 Simon D.Levy

  MIT License
--}

module AltitudeController where

import Types

altitudeController :: Double -> Double -> Double -> Double -> Double -> ClosedLoopController
altitudeController _target _kp_z _kp_dz _ki_dz _windupMax = 
    \_time -> \_state -> (Demands 1 0 0 0)
