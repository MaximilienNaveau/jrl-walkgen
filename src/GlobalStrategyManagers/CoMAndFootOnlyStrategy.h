/*! \file CoMAndFootOnlyStrategy.h
  \brief This object defines a global strategy object to generate 
  only foot, ZMP reference and CoM trajectories position every 5 ms.

  Copyright (c) 2007, 
  @author Francois Keith, Olivier Stasse
   
  JRL-Japan, CNRS/AIST
  
  All rights reserved.
  
  Please see License.txt for further information on license.   
*/

#include <SimplePlugin.h>
#include <PGTypes.h>

#include <MotionGeneration/ComAndFootRealization.h>
#include <GlobalStrategyManagers/GlobalStrategyManager.h>

#ifndef _COM_AND_FOOT_ONLY_STRATEGY_H_
#define _COM_AND_FOOT_ONLY_STRATEGY_H_
namespace PatternGeneratorJRL
{

  /** @ingroup pgjrl
      Implementation of the buffers handling without preview control
  */
  class  CoMAndFootOnlyStrategy: public GlobalStrategyManager
  {
    
  public:

    /*! Default constructor. */
    CoMAndFootOnlyStrategy(SimplePluginManager *aSimplePluginManager);

    /*! Default destructor. */
    ~CoMAndFootOnlyStrategy();

    /*! \name Reimplement the interface inherited from Global Strategy Manager 
     @{
    */

    /*! Perform a 5 ms step to generate the necessary information.
      \note{The meaning and the way to use this method depends on the child class}.

      @param[out] LeftFootPosition: The position of the Left Foot position.
      @param[out] RightFootPosition: The position of the Right Foot position.
      @param[out] ZMPRefPos: The ZMP position to be feed to the controller, in the waist 
      frame reference.
      @param[out] finalCOMState: returns CoM state position, velocity and acceleration.
      @param[out] CurrentConfiguration: The results is a state vector containing the articular positions.
      @param[out] CurrentVelocity: The results is a state vector containing the speed.
      @param[out] CurrentAcceleration: The results is a state vector containing the acceleration.
    */
    int OneGlobalStepOfControl(FootAbsolutePosition &LeftFootPosition,
			       FootAbsolutePosition &RightFootPosition,
			       MAL_VECTOR(,double) & ZMPRefPos,
			       COMState & finalCOMState,
			       MAL_VECTOR(,double) & CurrentConfiguration,
			       MAL_VECTOR(,double) & CurrentVelocity,
			       MAL_VECTOR(,double) & CurrentAcceleration);
    

    
    /*! Computes the COM of the robot with the Joint values given in BodyAngles,
      velocities set to zero, and returns the values of the COM in aStaringCOMState.
      Assuming that the waist is at (0,0,0)
      it returns the associate initial values for the left and right foot.
      @param[in] BodyAngles: 4x4 matrix of the robot's root (most of the time, the waist)
      pose (position + orientation).
      @param[out] aStartingCOMState: Returns the 3D position of the CoM for the current
      position of the robot.
      @param[out] aStartingZMPPosition: Returns the 3D position of the ZMP for the current
      position of the robot.
      @param[out] InitLeftFootPosition: Returns the position of the left foot in
      the waist coordinates frame.
      @param[out] InitRightFootPosition: Returns the position of the right foot
      in the waist coordinates frame.
    */
    int EvaluateStartingState(MAL_VECTOR( &,double) BodyAngles,
			      COMState & aStartingCOMState,
			      MAL_S3_VECTOR(& ,double) aStartingZMPPosition,
			      MAL_VECTOR(& ,double) aStartingWaistPose,
			      FootAbsolutePosition & InitLeftFootPosition,
			      FootAbsolutePosition & InitRightFootPosition);




    /*! This method returns :
      \li -1 if there is no more motion to realize 
      \li 0 if a new step is needed,
      \li 1 if there is still enough steps inside the internal stack.
    */
    int EndOfMotion();

    /*! @} */

    /*! Methods related to the end of the motion.
     @{ 
    */
    /* Fix the end of the buffer to be tested. */
    void SetTheLimitOfTheBuffer(unsigned int lBufferSizeLimit);
    
    /*! @} */


    /*! Reimplement the Call method for SimplePlugin part */
    void CallMethod(std::string &Method, std::istringstream &astrm);
    
    /*! */
    void Setup(deque<ZMPPosition> & aZMPPositions,
	       deque<COMState> & aCOMBuffer,
	       deque<FootAbsolutePosition> & aLeftFootAbsolutePositions,
	       deque<FootAbsolutePosition> & aRightFootAbsolutePositions);

    /*! \brief Initialization of the inter objects relationship. */
    int InitInterObjects(CjrlHumanoidDynamicRobot * aHDR,
			 ComAndFootRealization * aCFR,
			 StepStackHandler * aSSH);

  protected:
    /*! Count the number of successives hits on the bottom of the buffers. */
    int m_NbOfHitBottom;

    /*! Keeps a link towards an object allowing to find a pose for a given CoM and
      foot position. */
    ComAndFootRealization *m_ComAndFootRealization;

    /*! Set the position of the buffer size limit. */
    unsigned m_BufferSizeLimit;
  };
};
#endif /* _COM_AND_FOOT_ONLY_STRATEGY_H_ */
