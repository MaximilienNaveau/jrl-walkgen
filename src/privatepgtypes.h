/*
 * Copyright 2010,
 *
 * Andrei Herdt
 * Olivier  Stasse
 *
 * JRL, CNRS/AIST
 *
 * This file is part of walkGenJrl.
 * walkGenJrl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * walkGenJrl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Lesser Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with walkGenJrl.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Research carried out within the scope of the
 *  Joint Japanese-French Robotics Laboratory (JRL)
 */
/*! \file privatepgtypes.h
  \brief Defines basic types for the Humanoid Walking Pattern Generator.
*/

#ifndef _PATTERN_GENERATOR_INTERNAL_PRIVATE_H_
#define  _PATTERN_GENERATOR_INTERNAL_PRIVATE_H_

#include <jrl/mal/matrixabstractlayer.hh>

namespace PatternGeneratorJRL
{

  // Support state of the robot at a certain point in time
  struct support_state_s
  {
    int Phase, Foot, StepsLeft, StepNumber;
    bool SSSS, StateChanged;
    double TimeLimit;
  };
  typedef struct support_state_s support_state_t;

  // Support state of the robot at a certain point in time
  struct com_s
  {
    MAL_VECTOR(x,double);
    MAL_VECTOR(y,double);
    MAL_VECTOR(z,double);

    struct com_s & operator=(const com_s &aCS);

    void reset();

    com_s();
  };
  typedef struct com_s com_t;

  // Support state of the robot at a certain point in time
  struct trunk_s
  {
    MAL_VECTOR(x,double);
    MAL_VECTOR(y,double);
    MAL_VECTOR(z,double);

    MAL_VECTOR(yaw,double);
    MAL_VECTOR(pitch,double);
    MAL_VECTOR(roll,double);

    struct trunk_s & operator=(const trunk_s &aTS);

    void reset();

    trunk_s();
  };
  typedef struct trunk_s trunk_t;

  //State of the feet on the ground
  struct supportfoot_s
  {
    double x,y,theta,StartTime;
    int SupportFoot;
  };
  typedef struct supportfoot_s
    supportfoot_t;

  /// Absolute reference.
  struct reference_s
  {
    struct frame_s
    {
      /// \brief Constant reference
      double x,y, yaw;

      /// \brief Reference vectors
      MAL_VECTOR(X,double);
      MAL_VECTOR(Y,double);
      MAL_VECTOR(YAW,double);
    };
    typedef struct frame_s frame_t;

    frame_t global, local;

  };
  typedef struct reference_s reference_t;

}

#endif /* _PATTERN_GENERATOR_INTERNAL_PRIVATE_H_ */