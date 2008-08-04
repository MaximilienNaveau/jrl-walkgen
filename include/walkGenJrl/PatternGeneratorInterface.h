/*! \file PatternGeneratorInterface.h
    \brief  This object provides a unified interface to access the pattern generator.
    It allows to hide all the computation and hacking to the user.

    SVN Information:
   $Id$
   $Author$
   $Date$
   $Revision $
   $Source $
   $Log $


   Copyright (c) 2005-2006, 
   @author Olivier Stasse
   
   JRL-Japan, CNRS/AIST

   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without modification, 
   are permitted provided that the following conditions are met:
   
   * Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
   * Neither the name of the CNRS and AIST nor the names of its contributors 
   may be used to endorse or promote products derived from this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
   AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
   OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _PATTERN_GENERATOR_INTERFACE_H_
#define _PATTERN_GENERATOR_INTERFACE_H_

#include <sstream>


#include <MatrixAbstractLayer/MatrixAbstractLayer.h>

#include <dynamicsJRLJapan/HumanoidDynamicMultiBody.h>

#include <walkGenJrl/PreviewControl/ZMPPreviewControlWithMultiBodyZMP.h>
#include <walkGenJrl/PreviewControl/PreviewControl.h>

#include <walkGenJrl/ZMPRefTrajectoryGeneration/ZMPDiscretization.h>
#include <walkGenJrl/ZMPRefTrajectoryGeneration/ZMPQPWithConstraint.h>
#include <walkGenJrl/ZMPRefTrajectoryGeneration/AnalyticalMorisawaCompact.h>

#include <walkGenJrl/MotionGeneration/ComAndFootRealizationByGeometry.h>
#include <walkGenJrl/MotionGeneration/GenerateMotionFromKineoWorks.h>
#include <walkGenJrl/MotionGeneration/StepOverPlanner.h>

#include <walkGenJrl/FootTrajectoryGeneration/LeftAndRightFootTrajectoryGenerationMultiple.h>

#include <walkGenJrl/StepStackHandler.h>

#include <walkGenJrl/SimplePluginManager.h>

#include <walkGenJrl/GlobalStrategyManagers/DoubleStagePreviewControlStrategy.h>
#include <walkGenJrl/GlobalStrategyManagers/CoMAndFootOnlyStrategy.h>

namespace PatternGeneratorJRL
{
  class SimplePlugin;

  /** @ingroup Interface
      This class is the interface between the Pattern Generator and the 
      external world. In addition to the classical setter and getter for various parameters
      there is the possibility to pass commands a string of stream to the method
      \a ParseCmd().
   */
  class PatternGeneratorInterface : public SimplePluginManager
  {    
  public:
    
    /*! Constructor 
      @param strm: Should provide the file to initialize the preview control,
      the path to the VRML model, and the name of the file containing the VRML model.
     */
    PatternGeneratorInterface(std::istringstream &strm);

    /*! Destructor */
    ~PatternGeneratorInterface();

    
    /*! \name High levels function to create automatically stack of steps following specific motions. 
      @{
    */
    /*! \brief This methods generate a stack of steps which make the robot follows an arc.
      The direction of the robot is tangential to the arc.

      @param[in] x: Position of the center of the circle along the X-axis.
      @param[in] y: Position of the center of the circle along the Y-axis.
      @param[in] R: Ray of the circle.
      @param[in] arc_deg: Arc in degrees along which the robot walks.
      @param[in] SupportFoot: Indicates which is the first support foot (1) Left or (-1) Right.
     */
    void CreateArcInStepStack(  double x,
				double y,
				double R,
				double arc_deg,
				int SupportFoot);
    
    /*! \brief This methods generate a stack of steps which make the robot follows an arc.
      The direction of the robot is towards the center of the arc.
      The robot is therefore expected to move sideways.

      @param[in] R: Ray of the circle.
      @param[in] arc_deg: Arc in degrees along which the robot walks.
      @param[in] SupportFoot: Indicates which is the first support foot (1) Left or (-1) Right.
     */
    void CreateArcCenteredInStepStack( double R,
				       double arc_deg,
				       int SupportFoot);
    
    /*! \brief This specifies which foot will be used as the first support of the motion. */
    void PrepareForSupportFoot(int SupportFoot);
    
    /*! \brief This method precomputes all the buffers necessary for walking according to the chosen strategy. */
    void FinishAndRealizeStepSequence();
    /*! @} */


    /*! Common Initialization of walking. 
      @param[out] lStartingCOMPosition: For the starting position on the articular space, returns
      the COM position.
      @param[out] BodyAnglesIni: Basically it is a copy of CurrentJointValues but as a vector.
      @param[out] InitLeftFootAbsPos: Returns the current absolute position of the left foot for
      the given posture of the robot.
      @param[out] InitRightFootAbsPos: Returns the current absolute position of the right foot
      for the given posture of the robot.
      @param[out] lRelativeFootPositions: List of relative positions for the support foot still in the
      stack of steps.
      @param[in] lCurrentJointValues: The vector of articular values in classical C++ style.
      @param[in] ClearStepStackHandler: Clean the stack of steps after copy.
     */
    void CommonInitializationOfWalking(COMPosition & lStartingCOMPosition,
				       MAL_VECTOR(  & ,double) BodyAnglesIni,
				       FootAbsolutePosition & InitLeftFootAbsPos, 
				       FootAbsolutePosition & InitRightFootAbsPos,
				       deque<RelativeFootPosition> & lRelativeFootPositions,
				       vector<double> & lCurrentJointValues,
				       bool ClearStepStackHandler);


    /*! \name Methods for the control part. 
      @{
     */
    
    /*! \brief Run One Step of the global control loop aka The Main Method To Be Used.
     @param[out]  CurrentConfiguration The current configuration of the robot according to 
     the implementation of dynamic-JRLJapan. This should be first position and orientation
     of the waist, and then all the DOFs of your robot. 
     @param[out]  CurrentVelocity  The current velocity of the robot according to the 
     the implementation of dynamic-JRLJapan. 
     @param[out]  ZMPTarget  The target ZMP in the waist reference frame.
     @return True is there is still some data to send, false otherwise.
    */
    bool RunOneStepOfTheControlLoop(MAL_VECTOR(,double) & CurrentConfiguration,
				    MAL_VECTOR(,double) & CurrentVelocity,
				    MAL_VECTOR(,double) & ZMPTarget);

    /*! \brief Run One Step of the global control loop aka The Main Method To Be Used.
     @param[out]  CurrentConfiguration The current configuration of the robot according to 
     the implementation of dynamic-JRLJapan. This should be first position and orientation
     of the waist, and then all the DOFs of your robot. 
     @param[out]  CurrentVelocity  The current velocity of the robot according to the 
     the implementation of dynamic-JRLJapan. 
     @param[out]  ZMPTarget  The target ZMP in the waist reference frame.
     @param[out] COMPosition The CoM position for this motion.
     @param[out] LeftFootPosition: Absolute position of the left foot.
     @param[out] RightFootPosition: Absolute position of the right foot.
     @return True is there is still some data to send, false otherwise.
    */
    bool RunOneStepOfTheControlLoop(MAL_VECTOR(,double) & CurrentConfiguration,
				    MAL_VECTOR(,double) & CurrentVelocity,
				    MAL_VECTOR(,double) &ZMPTarget,
				    COMPosition &COMPosition,
				    FootAbsolutePosition &LeftFootPosition,
				    FootAbsolutePosition &RightFootPosition);

    /*! \brief Run One Step of the global control loop aka The Main Method To Be Used.
      @param[out] LeftFootPosition: Absolute position of the left foot.
      @param[out] RightFootPosition: Absolute position of the right foot.
      @param[out] ZMPRefPos: ZMP position new reference 
      @param[out] COMRefPos: COM position new reference.
      @return True is there is still some data to send, false otherwise.
    */
    bool RunOneStepOfTheControlLoop(FootAbsolutePosition &LeftFootPosition,
				    FootAbsolutePosition &RightFootPosition,
				    ZMPPosition &ZMPRefPos,
				    COMPosition &COMRefPos);
    /*! @} */

    /*! \brief Debug control loop */
    void DebugControlLoop(MAL_VECTOR(,double) & CurrentConfiguration,
			  MAL_VECTOR(,double) & CurrentVelocity,
			  int localindex);

    /*! Set the current joint values of the robot.
      This method is used to properly initialize the pattern generator.
      It also updates the state of the robot if other control mechanisms 
      modifies the upper body part and if this should be taken into account
      into the pattern generator in the second loop of control. */
    void SetCurrentJointValues(MAL_VECTOR( &lCurrentJointValues,double));

    /*! \brief Returns the walking mode. */
    int GetWalkMode();
    
    /*! \brief Get the leg joint velocity */
    void GetLegJointVelocity(MAL_VECTOR( &dqr,double), 
			     MAL_VECTOR( &dql,double));

    /*! \brief Read a sequence of steps. */
    void ReadSequenceOfSteps(istringstream &strm);
    
    /*! \name On-line steps related methods 
      @{
     */
    /*! \brief Start the creation of steps on line. */
    void StartOnLineStepSequencing();

    /*! \brief Start the creation of steps on line (istringstream interface). */
    void m_StartOnLineStepSequencing(istringstream & strm);
    
    /*! \brief Stop the creation of steps on line. */
    void StopOnLineStepSequencing();

    /*! \brief Stop the creation of steps on line (istringstream interface). */
    void m_StopOnLineStepSequencing(istringstream &strm2);

    
    /*! \brief Add an online step */
    void AddOnLineStep(double X, double Y, double Theta);

    /*! \brief Change online step.
      The strategy is the following: the step in single support phase at time t
      has its landing position changed to \f$ (X,Y,\theta) \f$ in absolute
      coordinates (i.e. in the world reference frame of the free flyer of the robot).
      For stability reason there is no guarantee that this method can 
      realized the operation. Please see the documentation of the walking pattern generator
      algorithm used. 
      
      If the time falls during a double support phase, the next single support phase is chosen.
	    
      @param[in] Time: Time information of the step. 
      @param[in] aFootAbsolutePosition: Absolute position of the foot.
      @return If the operation failed the method returns a negative number related
      to an error, 0 otherwise.
     */
    int ChangeOnLineStep(double Time, 
			 FootAbsolutePosition &aFootAbsolutePosition);

    /*! @} */

    /*! \name For SLAM
      @{ */
    /*! \brief Update the current waist absolute position */
    void UpdateAbsolutePosition(bool UpdateAbsMotionOrNot);

    /*! \brief Get the waist position and orientation as a quaternion,
     and the planar X-Y orientation in Orientation. */
    void getWaistPositionAndOrientation(double TQ[7],double &Orientation);

    /*! \brief Set Waist position and Orientation */
    void setWaistPositionAndOrientation(double TQ[7]);

    /*! \brief Get Waist velocity */
    void getWaistVelocity(double &dx,
			  double &dy,
			  double &omega) ;

    /*! \brief An other method to get the waist position using a matrix. */
    void getWaistPositionMatrix(MAL_S4x4_MATRIX( &lWaistAbsPos,double));
     
    /*!@} */

    /*! \name Handling of the inter-objects relationships.  
      @{
    */
    
    /*! \brief Instanciate the necessary objects. */
    void ObjectsInstanciation(string & HumanoidSpecificitiesFileName);


    /*! \brief Set the inter object relationship. */
    void InterObjectRelationInitialization(string & PCParametersFileName,
					   string & HumanoidVRMLFileDirectory,
					   string & HumanoidVRMLFileName,
					   string & HumanoidLinkJointRank);

    /*! @}*/

    /*! \name System to call a given method based on registration of a method. 
      @{
     */

    /*! \brief Parse a command (to be used out of a plugin) and call all objects which registered the method. */
    int ParseCmd(std::istringstream &strm);

    /*! \brief This method register a method to a specific object which derivates from SimplePlugin class. */
    bool RegisterMethod(string &MethodName, SimplePlugin *aSP);

    /*! @} */

    /*! \name Methods to be register in hrpsys. 
      @{
     */
    
    /*! \brief Set the gain factor for the default behavior of the arm */
    void m_SetArmParameters(std::istringstream &strm);

    /*! \brief Set obstacles parameters for stepping over */
    void m_SetObstacleParameters(std::istringstream &strm);

    /*! \brief Set the shift of the ZMP height for stepping over. */
    void m_SetZMPShiftParameters(std::istringstream &strm);
    
    /*! \brief Set time distribution parameters. */
    void m_SetTimeDistrParameters(std::istringstream &strm);

    /*! \brief Set upper body motion parameters. */
    void m_SetUpperBodyMotionParameters(std::istringstream &strm);

    /*! \brief Set the limits of the feasibility (stepping over parameters) */
    void m_SetLimitsFeasibility(std::istringstream &strm);

    /*! \brief Read file from Kineoworks. */
    void m_ReadFileFromKineoWorks(std::istringstream &strm);

    /*! \brief Specify a sequence of step without asking for 
      immediate execution and end sequence. */
    void m_PartialStepSequence(istringstream &strm);

    /*! \brief This method set PBW's algorithm for ZMP trajectory planning. */
    void m_SetAlgoForZMPTraj(istringstream &strm);

    /*! \brief Interface to hrpsys to start the realization of 
     the stacked of step sequences. */
    void m_FinishAndRealizeStepSequence(std::istringstream &strm);
    
    /*! \brief Realize a sequence of steps. */
    void m_StepSequence(std::istringstream &strm);

    /*! \brief Set the angle for lift off and landing of the foot. */
    void m_SetOmega(std::istringstream &strm);

    /*! \brief Set the maximal height of the foot trajectory. */
    void m_SetStepHeight(std::istringstream &strm);

    /*! @} */

  private:

    /*! Object to handle the stack of relative steps. */
    StepStackHandler *m_StepStackHandler;

    /*! Buffer needed to perform the stepping over
      obstacle. */
    vector<double> m_ZMPShift;
    
    /*! Gain factor for the default arm motion while walking. */
    double m_GainFactor;

    /*! Objects to generate a ZMP profile from
      the step of stacks. They provide a buffer for
      the ZMP position to be used every dt
      in the control loop. @{ */

    /*! Kajita's heuristic: the center of the convex hull. */
    ZMPDiscretization * m_ZMPD;

    /*! QP formulation with constraints. */
    ZMPQPWithConstraint * m_ZMPQP;
    
    /*! ZMP and CoM trajectories generation from an analytical formulation */
    AnalyticalMorisawaCompact * m_ZMPM;

    /*@} */

    /*! The Preview Control object. */
    PreviewControl *m_PC;
    
    /*! The object to be used to perform one step of
      control, and generates the corrected CoM trajectory. */
    ZMPPreviewControlWithMultiBodyZMP *m_ZMPpcwmbz;

    /*! Object needed to perform a path provided by Kineo */
    GenerateMotionFromKineoWorks *m_GMFKW;
    

    /*! Conversion between the index of the plan and the robot DOFs. */
    vector<int> m_ConversionForUpperBodyFromLocalIndexToRobotDOFs;
    
    /*! Current Actuated Joint values of the robot. */
    vector<double> m_CurrentActuatedJointValues;

    /*! Position of the waist: 
      Relative.*/
    MAL_S4x4_MATRIX( m_WaistRelativePos,double);
    
    /*! Absolute: */
    MAL_S4x4_MATRIX( m_WaistAbsPos,double);
    
    /*! Orientation: */
    double m_AbsTheta, m_AbsMotionTheta;
    
    /*! Position of the motion: */
    MAL_S4x4_MATRIX( m_MotionAbsPos,double);
    MAL_S4x4_MATRIX( m_MotionAbsOrientation,double);
    
    /*! Absolute speed:*/
    MAL_S4_VECTOR( m_AbsLinearVelocity,double);
    MAL_S4_VECTOR( m_AbsAngularVelocity,double);

    /*! Aboluste acceleration */
    MAL_S4_VECTOR( m_AbsLinearAcc,double);
    
    /*! Keeps track of the last correct support foot. */
    int m_KeepLastCorrectSupportFoot;
    
    /*! Boolean to ensure a correct initialization of the 
      step's stack. */
    bool m_IncorrectInitialization;

    /*! \name Global strategy handlers  
      @{
     */
    /*! \brief Double stage preview control strategy */
    DoubleStagePreviewControlStrategy * m_DoubleStagePCStrategy;

    /*! \brief Simple strategy just output CoM and Foot position. */
    CoMAndFootOnlyStrategy * m_CoMAndFootOnlyStrategy;

    /*! \brief General handler. */
    GlobalStrategyManager *m_GlobalStrategyManager;
    
    /*! @} */

    /*! Store the debug mode. */
    int m_DebugMode;

    /*! Store the number of degree of freedoms */
    int m_DOF;
    
    /*! Store the height of the arm. */
    double m_ZARM;

    /**! \name Time related parameters. 
       @{
     */
    /*! \brief Sampling period of the control loop. */
    double m_SamplingPeriod;

    /*! \brief Window of the preview control */
    double m_PreviewControlTime;
    
    /*! \brief Internal clock. 
      This field is updated every call to RunOneStepOfControl.
      It is assumed that this is done every m_SamplingPeriod.
     */
    double m_InternalClock;

    /*! @} */

    /*! Store the local Single support time,
      and the Double support time. */
    float m_TSsupport, m_TDsupport;

    /*! Height of the CoM. */
    double m_Zc;

    /*! Discrete size of the preview control window */
    unsigned int m_NL;
    
    /*! Local time while performing the control loop. */
    unsigned long int m_count;
    

    /*! Maximal value for the arms in front of the robot */
    double m_Xmax;      

    /*! Variables used to compute speed for other purposes. */
    MAL_VECTOR( m_prev_qr,double);
    MAL_VECTOR( m_prev_ql,double);
    MAL_VECTOR( m_prev_dqr, double);
    MAL_VECTOR( m_prev_dql, double);


    /* Debug variables. */
    MAL_VECTOR( m_Debug_prev_qr, double);
    MAL_VECTOR( m_Debug_prev_ql,double);
    MAL_VECTOR( m_Debug_prev_dqr, double);
    MAL_VECTOR( m_Debug_prev_dql,double);
    MAL_VECTOR( m_Debug_prev_UpperBodyAngles,double);
    MAL_VECTOR( m_Debug_prev_qr_RefState, double);
    MAL_VECTOR( m_Debug_prev_ql_RefState,double);

    double m_Debug_prev_qWaistYaw, m_Debug_prev_dqWaistYaw;
    MAL_S3_VECTOR(,double) m_Debug_prev_P, m_Debug_prev_L;
    bool m_FirstPrint, m_FirstRead;

    bool m_ShouldBeRunning;

    /*! To handle a new step. */
    bool m_NewStep;
    double m_NewStepX, m_NewStepY, m_NewTheta;

    /* ! Store the current relative state of the waist */
    COMPosition m_CurrentWaistState;
    
    /* ! current time period for the control */
    double m_dt;

    /*! \name Internals to deal with several ZMP CoM generation algorithms 
     @{ */
    /*! Algorithm to compute ZMP and CoM trajectory */
    int m_AlgorithmforZMPCOM;

    /*! Constants 
     @{ */
    /*! Using Preview Control with 2 stages proposed by Shuuji Kajita in 2003. */
    static const int ZMPCOM_KAJITA_2003=1;
    
    /*! Using the preview control with 2 stages proposed by Pierre-Brice Wieber in 2006. */
    static const int ZMPCOM_WIEBER_2006=2;

    /*! Using the analytical solution proposed by Morisawa in 2007. */
    static const int ZMPCOM_MORISAWA_2007=3;
    
    /*! @} */
    /*! @} */

    /*! Humanoid Dynamic robot */
    HumanoidDynamicMultiBody * m_HumanoidDynamicRobot, * m_2HumanoidDynamicRobot;

    /*! Speed of the leg. */
    MAL_VECTOR(,double) m_dqr,m_dql;

    /*! Objet to realize the generate the posture for given CoM
      and feet positions. */
    ComAndFootRealization * m_ComAndFootRealization;

    /* \name Object related to stepping over. 
       @{
     */
    
    /*! Planner for stepping over an obstacle. */
    StepOverPlanner *m_StOvPl;

    /*! Position and parameters related to the obstacle. */
    ObstaclePar m_ObstaclePars;
    
    /*! Boolean on the obstacle's detection */
    bool m_ObstacleDetected;	

    /*! Time Distribution factor */
    vector<double> m_TimeDistrFactor;

    /*! Variable for delta feasibility limit */
    double m_DeltaFeasibilityLimit;	
    
    /* @} */

    /*! \brief Foot Trajectory Generator */
    LeftAndRightFootTrajectoryGenerationMultiple * m_FeetTrajectoryGenerator;

    
    /*! \name Buffers of Positions. 
      @{
     */
   
    /*! Buffer of ZMP positions */
    deque<ZMPPosition> m_ZMPPositions;

    /*! Buffer of Absolute foot position (World frame) */
    deque<FootAbsolutePosition> m_FootAbsolutePositions;

    /*! Buffer of absolute foot position. */
    deque<FootAbsolutePosition> m_LeftFootPositions, m_RightFootPositions;
    
    /*! Buffer for the COM position. */
    deque<COMPosition> m_COMBuffer;
    
    /*! @} */


  protected:

    /*! \name Internal methods which are not to be exposed. 
      They are therefore subject to change. 
      @{
     */
    
    /*! \brief Expansion of the buffers handling Center of Masse positions,
      as well as Upper Body Positions. */
    void ExpandCOMPositionsQueues(int aNumber);

    /*! \brief Compute the COM, left and right foot position for a given BodyAngle position */
    void EvaluateStartingCOM(MAL_VECTOR(  & Configuration,double),
			     MAL_S3_VECTOR(  & lStartingCOMPosition,double));

    /*! \brief Fill the internal buffer with the appropriate information depending on the strategy. 
    The behavior of this method depends on \a m_AlgorithmforZMPCOM.
    */
    int CreateZMPReferences(deque<RelativeFootPosition> &lRelativeFootPositions,
			    COMPosition &lStartingCOMPosition,
			    FootAbsolutePosition & InitLeftFootAbsPos,
			    FootAbsolutePosition & InitRightFootAbsPos);
    /* @} */
  };

};


#endif
