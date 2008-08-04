/* \file AnalyticalMorisawaAbstract.h
   \brief This abstract class specifies how to generate the reference value for the
   ZMP based on a polynomial representation following the paper
   "Experimentation of Humanoid Walking Allowing Immediate
   Modification of Foot Place Based on Analytical Solution"
   Morisawa, Harada, Kajita, Nakaoka, Fujiwara, Kanehiro, Hirukawa, 
   ICRA 2007, 3989--3994

   Copyright (c) 2007, 
   @author Olivier Stasse,
   
   JRL-Japan, CNRS/AIST

   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without modification, 
   are permitted provided that the following conditions are met:
   
   * Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
   * Neither the name of the CNRS/AIST nor the names of its contributors 
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
#ifndef _ANALYTICAL_MORISAWA_ABSTRACT_H_
#define _ANALYTICAL_MORISAWA_ABSTRACT_H_

#include <dynamicsJRLJapan/HumanoidSpecificities.h>

#include <walkGenJrl/Mathematics/PolynomeFoot.h>
#include <walkGenJrl/Mathematics/ConvexHull.h>
#include <walkGenJrl/Mathematics/AnalyticalZMPCOGTrajectory.h>
#include <walkGenJrl/PreviewControl/PreviewControl.h>
#include <walkGenJrl/ZMPRefTrajectoryGeneration/ZMPRefTrajectoryGeneration.h>



namespace PatternGeneratorJRL
{
  /*! \brief Class to define the abstract interface to compute analytically the trajectories of both the ZMP and the CoM.
    @ingroup analyticalformulation

    Using this method we assume again the linear inverted pendulum of Kajita,
    and therefore assume that 
    
   */
  class AnalyticalMorisawaAbstract: public ZMPRefTrajectoryGeneration
    {
      
    public:

      const static unsigned int SINGLE_SUPPORT=0;
      const static unsigned int DOUBLE_SUPPORT=1;

      /*! Constructor */
      AnalyticalMorisawaAbstract(SimplePluginManager * lSPM);
      
      /*! Destructor */
      virtual ~AnalyticalMorisawaAbstract();

      /*! \name Methods inherited from ZMPRefTrajectoryGeneration and reimplemented
	@{ */

      /*! Returns the CoM and ZMP trajectory for some relative foot positions. 
	Generate ZMP discreatization from a vector of foot position.
	ASSUME A COMPLETE MOTION FROM END TO START, and GENERATE EVERY VALUE.
	
	@param[out] ZMPPositions: Returns the ZMP reference values for the overall motion.
	Those are absolute position in the world reference frame. The origin is the initial
	position of the robot. The relative foot position specified are added.
	
	@param[out] CoMPositions: Returns the CoM reference values for the overall motion.
	Those are absolute position in the world reference frame. The origin is the initial
	position of the robot. The relative foot position specified are added.

	@param[in] RelativeFootPositions: The set of 
	relative steps to be performed by the robot.
			
	@param[out] LeftFootAbsolutePositions: Returns the absolute position of the left foot.
	According to the macro FULL_POLYNOME the trajectory will follow a third order
	polynom or a fifth order. By experience it is wise to put a third order. 
	A null acceleration might cause problem for the compensation of the Z-axis momentum.
	
	@param[out] RightFootAbsolutePositions: Returns the absolute position of the right foot.
		
	@param[in] Xmax: The maximal distance of a hand along the X axis in the waist coordinates.

	@param[in] lStartingCOMPosition: The starting position of the CoM.
	
	@param[in] InitLeftFootAbsolutePosition: The absolute position of the left foot.

	@param[in] InitRightFootAbsolutePosition: The absolute position of the right foot.

      */
      virtual void GetZMPDiscretization(deque<ZMPPosition> & ZMPPositions,
					deque<COMPosition> & CoMPositions,
					deque<RelativeFootPosition> &RelativeFootPositions,
					deque<FootAbsolutePosition> &LeftFootAbsolutePositions,
					deque<FootAbsolutePosition> &RightFootAbsolutePositions,
					double Xmax,
					COMPosition & lStartingCOMPosition,
					FootAbsolutePosition & InitLeftFootAbsolutePosition,
					FootAbsolutePosition & InitRightFootAbsolutePosition) =0;
      
      /*! Methods for on-line generation. (First version)
	The queues will be updated as follows:
	\li \c The first values necessary to start walking will be inserted.
	\li \c The initial positions of the feet will be taken into account
	according to InitLeftFootAbsolutePosition and InitRightFootAbsolutePosition.
	\li \c The RelativeFootPositions stack will be taken into account,
	\li \c The starting COM Position.
	Returns the number of steps which has been completely put inside 
	the queue of ZMP, and foot positions.
      */
      virtual int InitOnLine(deque<ZMPPosition> & FinalZMPPositions,
			     deque<COMPosition> & CoMPositions,
			     deque<FootAbsolutePosition> & FinalLeftFootAbsolutePositions,
			     deque<FootAbsolutePosition> & FinalRightFootAbsolutePositions,
			     FootAbsolutePosition & InitLeftFootAbsolutePosition,
			     FootAbsolutePosition & InitRightFootAbsolutePosition,
			     deque<RelativeFootPosition> &RelativeFootPositions,
			     COMPosition & lStartingCOMPosition) =0 ;
      
      /* ! \brief Method to update the stacks on-line */
      virtual void OnLine(double time,
			  deque<ZMPPosition> & FinalZMPPositions,		
			  deque<COMPosition> & CoMPositions,			     
			  deque<FootAbsolutePosition> &FinalLeftFootAbsolutePositions,
			  deque<FootAbsolutePosition> &FinalRightFootAbsolutePositions)=0;
      
      /* ! Methods to update the stack on-line by inserting a new foot position. */
      virtual void OnLineAddFoot(RelativeFootPosition & NewRelativeFootPosition,
				 deque<ZMPPosition> & FinalZMPPositions,	
				 deque<COMPosition> & CoMPositions,			  
				 deque<FootAbsolutePosition> &FinalLeftFootAbsolutePositions,
				 deque<FootAbsolutePosition> &FinalRightFootAbsolutePositions,
				 bool EndSequence)=0;

      /* ! \brief Method to change on line the landing position of a foot.
	 @return If the method failed it returns -1, 0 otherwise.
      */
      virtual int OnLineFootChange(double time,
				   FootAbsolutePosition &aFootAbsolutePosition,
				   deque<ZMPPosition> & FinalZMPPositions,			     
				   deque<COMPosition> & CoMPositions,
				   deque<FootAbsolutePosition> &FinalLeftFootAbsolutePositions,
				   deque<FootAbsolutePosition> &FinalRightFootAbsolutePositions,
				   StepStackHandler *aStepStackHandler)=0;

      /*! @} */
      
      /*! \name Methods specifics to our current implementation.
       @{ */
      /*! Set the number of steps in advance
	@return: true if everything went fine, false otherwise.
       */
      bool SetNumberOfStepsInAdvance(int lNumberOfStepsInAdvance);
      
      /*! Get the number of steps in advance.
	@return: return the number of intervals.
      */
      int GetNumberOfStepsInAdvance();

      /*! Get the number of intervals. */
      int GetNumberOfIntervals();

      /*! Get the \f[ \omega_j \f] */
      int GetOmegaj(vector<double> &GetOmegaj);

      /*! Get the \f[ \Delta T_j \f] */
      int GetDeltaTj(vector<double> &GetDeltaTj);

      /*! Set the \f[ \Delta T_j \f] */
      int SetDeltaTj(vector<double> &GetDeltaTj);

      /*! \name Methods for the full linear system 
	@{ */

      /*! \brief Building the Z matrix to be inverted. 
	@param lCoM: Profile of CoM for each interval.
	@param lZMP: Profile of ZMP for each interval.
       */
      virtual void BuildingTheZMatrix(vector<double> &lCoM, vector<double> &lZMP )=0;

      /*! \brief Building the w vector. 
	It is currently assume that all ZMP's speed will be
	set to zero, as well as the final COM's speed.
	The sequence of ZMPSequence is the final value of the
	ZMP. As a special case, the first interval being set
	as a single support phase
	@param[in] InitialCoMPos: Initial position of the CoM.
	@param[in] InitialComSpeed: Initial speed of the CoM.
	@param[in] ZMPPosSequence: Vector of position for each interval.
	@param[in] FinalCoMPos: The final CoM position of the trajectory.
	@param[in] aAZCT: The analytical trajectory from which some coefficients
	have to be extracted to compute appropriatly the vector \f[ w \f] .
       */
      virtual void ComputeW(double InitialCoMPos,
			    double InitialComSpeed,
			    vector<double> &ZMPPosSequence,
			    double FinalCoMPos,
			    AnalyticalZMPCOGTrajectory & aAZCT) =0 ;

      /*! \brief Transfert the computed weights to an Analytical ZMP COG
	Trajectory able to generate the appropriate intermediates values.
	@param aAZCT: The object to be filled with the appropriate intermediate values.
	@param lCoMZ: The height trajectory of the CoM.
	@param lZMPZ: The height trajectory of the ZMP.
	@param lZMPInit: The initial value of the ZMP for this trajectory along the axis.
	@param lZMPEnd: The final value of the ZMP for this trajectory along the axis.
	@param InitializeaAZCT: If set to true this variable trigger the initialization
	of the object aAZCT, does nothing otherwise.
      */
      virtual void TransfertTheCoefficientsToTrajectories(AnalyticalZMPCOGTrajectory &aAZCT,
							  vector<double> &lCoMZ,
							  vector<double> &lZMPZ,
							  double &lZMPInit,
							  double &lZMPEnd,
							  bool InitializeaAZCT) =0 ;

      /*! @} */

      /*! \brief Initialize automatically Polynomial degrees, and temporal intervals. 
	@return True if succeedeed, false otherwise.
       */
      virtual bool InitializeBasicVariables()=0;

      /*! \brief Get the weights for the polynomials. */
      bool GetPolynomialWeights(vector<double> &PolynomialWeights);

      /*! \brief Compute the polynomial weights. */
      virtual void ComputePolynomialWeights()=0;
      

      /*! \brief Get the polynomial degrees for the trajectory designed with
	this method. */
      void GetPolynomialDegrees(vector<unsigned int> &lPolynomialDegrees);
	
      friend std::ostream& operator <<(std::ostream &os,const AnalyticalMorisawaAbstract &obj);

      /*! \brief Compute the preview control time window. */
      void ComputePreviewControlTimeWindow()
      {
	m_PreviewControlTime = 0.0;
	for(unsigned int i=0;i<m_DeltaTj.size();i++)
	  m_PreviewControlTime += m_DeltaTj[i];
      }

      /*! @} */

    protected:
      /*! \name Store the matrices used for compution. 
	@{ 
      */

      /*! One of the most important matrix which stores all the temporal
	relationship between the intervals. */
      MAL_MATRIX(,double) m_Z;


      /*! The matrix used to specify the conditions of the intervals. */
      MAL_VECTOR(,double) m_w;

      /*! The matrix used to store the solution of the linear equation. */
      MAL_VECTOR(,double) m_y;

      /*! @} */
      
      /*! \name Control variables. 
	@{. */
      
      /*! \brief Number of steps ($NbSteps$) on which the polynomials coefficients are computed.
       */
      int m_NumberOfStepsInAdvance; 
      
      /*! Set of number of intervals.
	The relationship with the number of steps in advance is : 
	2 * m_NumberOfStepsInAdvance */
      int m_NumberOfIntervals;

      /*! \brief Set of temporal intervals. */
      vector<double> m_DeltaTj;

      /*! \brief Set of step types: 0 single support, 1 double support*/
      vector<unsigned int> m_StepTypes;

      /*! Vector build upon the CoM and ZMP profile along the Z axis */
      vector<double> m_Omegaj;

      /*! \brief Set of polynomial degrees. 
	The current strategy is to have the first and the last ones set to 4,
	while the intermediate intervals uses a 3rd order polynomial.

       */
      vector<unsigned int> m_PolynomialDegrees;
	      
      /*! \name Internal Methods to compute the full linear
	system. 
	@{
      */

      /*! \brief Building the Z1 Matrix */
      virtual void ComputeZ1(unsigned int &lindex) =0 ;
      
      /*! \brief Building the Zj Matrix 
       @param[in] intervalindex: Index of the interval, 
       @param[in] colindex: Index of the column inside the matrix,
       @param[in] rowindex: Index of the row inside the matrix. */
      virtual void ComputeZj(unsigned int intervalindex, 
			     unsigned int &colindex, 
			     unsigned int &rowindex) =0;
      
      /*! \brief Building the Zm Matrix 
	@param[in] intervalindex: Index of the interval, 
       @param[in] colindex: Index of the column inside the matrix,
       @param[in] rowindex: Index of the row inside the matrix.
       */
      virtual void ComputeZm(unsigned int intervalindex, 
			     unsigned int &colindex, 
			     unsigned int &rowindex)=0;

      /*! @} */

      /*! \brief Absolute Reference time */
      double m_AbsoluteTimeReference;
      

      /*! @} */

      /*! \brief Reference to the Humanoid Specificities. */
      dynamicsJRLJapan::HumanoidSpecificities * m_HS;
	      
      
      /*! \name Debugging fields 
	@{
       */
      /*! \brief Verbose level for debugging purposes.*/
      unsigned m_VerboseLevel;

      /*! @} */

    public:

      /*! \brief Get the absolute reference time of 
	the system */
      double  GetAbsoluteTimeReference() const
      { return m_AbsoluteTimeReference; }
      
      /*! \brief Set the absolute reference time of 
	the system */
      void SetAbsoluteTimeReference(double anAbsoluteTimeReference)
      { m_AbsoluteTimeReference = anAbsoluteTimeReference; }

      /*! \brief Get the reference to the object handling the
	humanoid specificities */
      dynamicsJRLJapan::HumanoidSpecificities * GetHumanoidSpecificites() const
	{ return m_HS;};

      /*! \brief Set the reference of the object handling the 
	humanoid specificities */
      void SetHumanoidSpecificities(HumanoidSpecificities *aHS)
        {  m_HS = aHS;   };
    };
};
#endif
