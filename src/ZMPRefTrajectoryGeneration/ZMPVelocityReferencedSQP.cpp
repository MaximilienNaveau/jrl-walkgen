/*
 * Copyright 2010,
 *
 *
 * Maximilien Naveau
 * Olivier Stasse
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

/*! This object generate all the values for the foot trajectories,
  and the desired ZMP based on a sequence of steps following a SQP
  formulation using QPoases solver as proposed by Naveau "adding paper citation"

  Maximilien Naveau,
  Olivier Stasse,
 */

#include "portability/gettimeofday.hh"

#ifdef WIN32
# include <Windows.h>
#endif /* WIN32 */

#include <time.h>
#include <iostream>
#include <fstream>

#include <privatepgtypes.hh>
#include <ZMPRefTrajectoryGeneration/ZMPVelocityReferencedSQP.hh>

#include <Debug.hh>

using namespace std;
using namespace PatternGeneratorJRL;

ZMPVelocityReferencedSQP::ZMPVelocityReferencedSQP(SimplePluginManager *SPM,
                                                 string , CjrlHumanoidDynamicRobot *aHS ) :
ZMPRefTrajectoryGeneration(SPM),OFTG_(NULL),dynamicFilter_(NULL)
{

//  NMPC_generator nmpc_generator (aSPM,aHRP2HDR) ;
//  vector<double>local_vel_ref(3,0.0);
//  local_vel_ref[0] = 0.2 ;
//  local_vel_ref[1] = 0.0 ;
//  local_vel_ref[2] = 0.2 ;
//  nmpc_generator.initNMPC_generator();

//  for(unsigned i=0 ; i<10 ; ++i)
//  {
//    nmpc_generator.setVelocityReference(local_vel_ref);
//    nmpc_generator.solve();
//  }
  // PG management
  Running_ = false ;
  TimeBuffer_ = 0.04 ;
  SQP_T_ = 0.1 ;
  SQP_N_ = 16 ;
  m_SamplingPeriod = 0.005 ;

  // Generator Management
  InterpolationPeriod_ = SQP_T_/2;
  previewDuration_ = 1.4 ;
  NbSampleControl_ = (int)round(SQP_T_/m_SamplingPeriod) ;
  NbSampleInterpolation_ = (int)round(SQP_T_/InterpolationPeriod_) ;
  previewSize_ = SQP_N_ ;
  StepPeriod_ = 0.8 ;
  SSPeriod_ = 0.7 ;
  DSPeriod_ = 0.1 ;
  FeetDistance_ = 0.2 ;
  CoMHeight_ = 0.814 ;
  UpperTimeLimitToUpdate_ = 0.0 ;

  // perturbation management
  PerturbationOccured_ = false ;
  RobotMass_ = aHS->mass() ;

  // interpolation management
  StepHeight_ = 0.05 ;
  CurrentIndex_ = 0 ;

  // Save the reference to HDR
  HDR_ = aHS ;

  // Initialize  the 2D LIPM
  LIPM_.SetSimulationControlPeriod( SQP_T_ );
  LIPM_.SetRobotControlPeriod( m_SamplingPeriod );
  LIPM_.InitializeSystem();

  // Create and initialize online interpolation of feet trajectories:
  // ----------------------------------------------------------------
  OFTG_ = new OnLineFootTrajectoryGeneration(SPM,HDR_->leftFoot());
  OFTG_->InitializeInternalDataStructures();
  OFTG_->SetSingleSupportTime( SSPeriod_ );
  OFTG_->SetDoubleSupportTime( DSPeriod_ );
  OFTG_->SetSamplingPeriod( m_SamplingPeriod );
  OFTG_->QPSamplingPeriod( SQP_T_ );
  OFTG_->NbSamplingsPreviewed( SQP_N_ );
  OFTG_->FeetDistance( FeetDistance_ );
  OFTG_->StepHeight( StepHeight_ );
  OFTG_->SetStepStairOn(0) ;

  NMPCgenerator_ = new NMPCgenerator(SPM,aHS);

  dynamicFilter_ = new DynamicFilter(SPM,aHS);

  // Register method to handle
  const unsigned int NbMethods = 3;
  string aMethodName[NbMethods] =
  {":previewcontroltime",
   ":numberstepsbeforestop",
   ":stoppg"
   ":setfeetconstraint"};

  for(unsigned int i=0;i<NbMethods;i++)
  {
    if (!RegisterMethod(aMethodName[i]))
    {
      std::cerr << "Unable to register " << aMethodName << std::endl;
    }
  }

  // init of the buffer for the kajita's dynamic filter

  // size = numberOfIterationOfThePreviewControl * NumberOfSample + Margin
  ZMPTraj_deq_.resize( SQP_N_ * NbSampleInterpolation_+10);
  COMTraj_deq_.resize( SQP_N_ * NbSampleInterpolation_+10);
  LeftFootTraj_deq_.resize( SQP_N_ * NbSampleInterpolation_+10) ;
  RightFootTraj_deq_.resize( SQP_N_ * NbSampleInterpolation_+10) ;

  ZMPTraj_deq_ctrl_.resize( SQP_N_ * NbSampleControl_+10) ;
  COMTraj_deq_ctrl_.resize( SQP_N_ * NbSampleControl_+10) ;
  LeftFootTraj_deq_ctrl_ .resize( SQP_N_ * NbSampleControl_+10) ;
  RightFootTraj_deq_ctrl_.resize( SQP_N_ * NbSampleControl_+10) ;
}


ZMPVelocityReferencedSQP::~ZMPVelocityReferencedSQP()
{
  if (NMPCgenerator_!=NULL)
  {
    delete NMPCgenerator_;
    NMPCgenerator_= NULL ;
  }
  if (OFTG_!=NULL)
  {
    delete OFTG_;
    OFTG_ = NULL ;
  }
  if (OFTG_!=NULL)
  {
    delete OFTG_;
    OFTG_ = NULL ;
  }
  if (dynamicFilter_!=NULL)
  {
    delete dynamicFilter_;
    dynamicFilter_ = NULL ;
  }
}

void ZMPVelocityReferencedSQP::setCoMPerturbationForce(istringstream &strm)
{

  MAL_VECTOR_RESIZE(PerturbationAcceleration_,6);

  strm >> PerturbationAcceleration_(2);
  strm >> PerturbationAcceleration_(5);
  PerturbationAcceleration_(2) = PerturbationAcceleration_(2)/RobotMass_;
  PerturbationAcceleration_(5) = PerturbationAcceleration_(5)/RobotMass_;
  PerturbationOccured_ = true;
}

void ZMPVelocityReferencedSQP::setCoMPerturbationForce(double x, double y)
{

  MAL_VECTOR_RESIZE(PerturbationAcceleration_,6);

  PerturbationAcceleration_(2) = x/RobotMass_;
  PerturbationAcceleration_(5) = y/RobotMass_;
  PerturbationOccured_ = true;
}

//--------------------------------------
//
//
//-----------new functions--------------
void ZMPVelocityReferencedSQP::CallMethod(std::string & Method, std::istringstream &strm)
{
  if (Method==":previewcontroltime")
  {
    strm >> m_PreviewControlTime;
  }
  if (Method==":numberstepsbeforestop")
  {
    support_state_t & CurrentSupport = NMPCgenerator_->currentSupport();
    strm >> CurrentSupport.NbStepsLeft;
    NMPCgenerator_->setNbStepsLeft(CurrentSupport.NbStepsLeft);
  }
  if (Method==":stoppg")
  {
    EndingPhase_ = true;
  }
  if(Method==":setfeetconstraint")
  {
   //RFI_->CallMethod(Method,strm); need a callMethod in NMPCgenerator_ class
  }
  ZMPRefTrajectoryGeneration::CallMethod(Method,strm);
}

int ZMPVelocityReferencedSQP::InitOnLine(deque<ZMPPosition> & FinalZMPTraj_deq,
                                        deque<COMState> & FinalCoMPositions_deq,
                                        deque<FootAbsolutePosition> & FinalLeftFootTraj_deq,
                                        deque<FootAbsolutePosition> & FinalRightFootTraj_deq,
                                        FootAbsolutePosition & InitLeftFootAbsolutePosition,
                                        FootAbsolutePosition & InitRightFootAbsolutePosition,
                                        deque<RelativeFootPosition> &, // RelativeFootPositions,
                                        COMState & lStartingCOMState,
                                        MAL_S3_VECTOR_TYPE(double) & lStartingZMPPosition)
{
  UpperTimeLimitToUpdate_ = 0.0;
  FootAbsolutePosition CurrentLeftFootAbsPos, CurrentRightFootAbsPos;

  // Set the internal state of the ZMPRefTrajectory object.
  m_OnLineMode = true;
  EndingPhase_ = false;
  TimeToStopOnLineMode_ = -1.0;

  // INITIALIZE FEET POSITIONS:
  // --------------------------
  CurrentLeftFootAbsPos = InitLeftFootAbsolutePosition;
  CurrentRightFootAbsPos = InitRightFootAbsolutePosition;

  // FILL THE QUEUES:
  // ----------------
  int AddArraySize;
  {
    assert(m_SamplingPeriod > 0);
    double ldAddArraySize = TimeBuffer_/m_SamplingPeriod;
    AddArraySize = (int)ldAddArraySize;
  }

  FinalZMPTraj_deq.resize(AddArraySize);
  FinalCoMPositions_deq.resize(AddArraySize);
  FinalLeftFootTraj_deq.resize(AddArraySize);
  FinalRightFootTraj_deq.resize(AddArraySize);
  int CurrentZMPindex=0;
  m_CurrentTime = 0.0;
  for( unsigned int i=0;i<FinalZMPTraj_deq.size();i++ )
  {
    // Smooth ramp
    FinalZMPTraj_deq[CurrentZMPindex].px = lStartingZMPPosition(0);
    FinalZMPTraj_deq[CurrentZMPindex].py = lStartingZMPPosition(1);
    FinalZMPTraj_deq[CurrentZMPindex].pz = lStartingZMPPosition(2);
    FinalZMPTraj_deq[CurrentZMPindex].theta = 0.0;
    FinalZMPTraj_deq[CurrentZMPindex].time = m_CurrentTime;
    FinalZMPTraj_deq[CurrentZMPindex].stepType = 0;

    // Set CoM positions.
    FinalCoMPositions_deq[CurrentZMPindex] = lStartingCOMState;
    // Set Left Foot positions.
    FinalLeftFootTraj_deq[CurrentZMPindex]  = CurrentLeftFootAbsPos;
    FinalRightFootTraj_deq[CurrentZMPindex] = CurrentRightFootAbsPos;
    FinalLeftFootTraj_deq[CurrentZMPindex].time =
        FinalRightFootTraj_deq[CurrentZMPindex].time = m_CurrentTime;
    FinalLeftFootTraj_deq[CurrentZMPindex].stepType =
        FinalRightFootTraj_deq[CurrentZMPindex].stepType = 10;

    m_CurrentTime += m_SamplingPeriod;
    CurrentZMPindex++;
  }

  // INITIAL SUPPORT STATE:
  // ----------------------
  support_state_t currentSupport ;
  currentSupport.Phase = DS; // TODO make the constraint in jerk working with stanfind still
  currentSupport.Foot = LEFT;
  currentSupport.TimeLimit = 0.9;
  currentSupport.NbStepsLeft = 0;
  currentSupport.StateChanged = false;
  currentSupport.X=0.00949035;
  currentSupport.Y=0.095;
  currentSupport.Yaw=0.0;
  currentSupport.StartTime = 0.0;

  NMPCgenerator_->initNMPCgenerator(currentSupport,
                                    lStartingCOMState,
                                    VelRef_);

  // INITIALIZE CENTER OF MASS:
  // --------------------------
  com_t CoM;
  CoM.x[0] = lStartingCOMState.x[0];
  CoM.x[1] = lStartingCOMState.x[1];
  CoM.x[2] = lStartingCOMState.x[2];
  CoM.y[0] = lStartingCOMState.y[0];
  CoM.y[1] = lStartingCOMState.y[1];
  CoM.y[2] = lStartingCOMState.y[2];
  CoM.z[0] = lStartingCOMState.z[0];
  CoM.z[1] = lStartingCOMState.z[1];
  CoM.z[2] = lStartingCOMState.z[2];
  itCoM = lStartingCOMState;
  LIPM_.SetComHeight(lStartingCOMState.z[0]);
  LIPM_.InitializeSystem();
  LIPM_(CoM);

  dynamicFilter_->getComAndFootRealization()->ShiftFoot(true);
  dynamicFilter_->init(m_SamplingPeriod,
                       InterpolationPeriod_,
                       SQP_T_,
                       SQP_N_*SQP_T_ ,
                       previewDuration_,
                       lStartingCOMState);
  return 0;
}



void ZMPVelocityReferencedSQP::OnLine(double time,
                                    deque<ZMPPosition> & FinalZMPTraj_deq,
                                    deque<COMState> & FinalCOMTraj_deq,
                                    deque<FootAbsolutePosition> & FinalLeftFootTraj_deq,
                                    deque<FootAbsolutePosition> & FinalRightFootTraj_deq)

{
  // If on-line mode not activated we go out.
  if (!m_OnLineMode)
  {
    return;
  }

  // Test if the end of the online mode has been reached.
  if ((EndingPhase_) &&
      (time>=TimeToStopOnLineMode_))
  { m_OnLineMode = false; }

  // UPDATE WALKING TRAJECTORIES:
  // ----------------------------
  if(time + 0.00001 > UpperTimeLimitToUpdate_)
  {

    // UPDATE INTERNAL DATA:
    // ---------------------
    VelRef_=NewVelRef_;
    NMPCgenerator_->updateInitialCondition(
        time,
        FinalLeftFootTraj_deq.back(),
        FinalRightFootTraj_deq.back(),
        itCoM,
        VelRef_);

    // SOLVE PROBLEM:
    // --------------
    NMPCgenerator_->solve();

    // INITIALIZE INTERPOLATION:
    // ------------------------
    CurrentIndex_ = FinalCOMTraj_deq.size();
    for (unsigned int i = 0  ; i < CurrentIndex_ ; ++i )
    {
        ZMPTraj_deq_ctrl_[i] = FinalZMPTraj_deq[i] ;
        COMTraj_deq_ctrl_[i] = FinalCOMTraj_deq[i] ;
        LeftFootTraj_deq_ctrl_ [i]= FinalLeftFootTraj_deq  [i] ;
        RightFootTraj_deq_ctrl_[i]= FinalRightFootTraj_deq [i] ;
    }
    FinalZMPTraj_deq.resize( NbSampleControl_ + CurrentIndex_ );
    FinalCOMTraj_deq.resize( NbSampleControl_ + CurrentIndex_ );
    FinalLeftFootTraj_deq .resize(NbSampleControl_ + CurrentIndex_);
    FinalRightFootTraj_deq.resize(NbSampleControl_ + CurrentIndex_);

    // INTERPOLATION
    // ------------------------
    // Compute the full trajectory in the preview window
    FullTrajectoryInterpolation(time);

    // Take only the data that are actually used by the robot
    for(unsigned i=0 ; i<NbSampleControl_ + CurrentIndex_ ; ++i)
    {
      FinalZMPTraj_deq      [i] = ZMPTraj_deq_ctrl_      [i] ;
      FinalCOMTraj_deq      [i] = COMTraj_deq_ctrl_      [i] ;
      FinalLeftFootTraj_deq [i] = LeftFootTraj_deq_ctrl_ [i] ;
      FinalRightFootTraj_deq[i] = RightFootTraj_deq_ctrl_[i] ;
    }

    bool filterOn_ = false ;
    if(filterOn_)
    {

      dynamicFilter_->OnLinefilter(COMTraj_deq_,ZMPTraj_deq_ctrl_,
                                   LeftFootTraj_deq_,
                                   RightFootTraj_deq_,
                                   deltaCOMTraj_deq_);

      //#define DEBUG
      #ifdef DEBUG
        dynamicFilter_->Debug(COMTraj_deq_ctrl_,
                              LeftFootTraj_deq_ctrl_,
                              RightFootTraj_deq_ctrl_,
                              COMTraj_deq_,ZMPTraj_deq_ctrl_,
                              LeftFootTraj_deq_,
                              RightFootTraj_deq_,
                              deltaCOMTraj_deq_);
      #endif
      // Correct the CoM.
      for (unsigned int i = 0 ; i < NbSampleControl_ ; ++i)
      {
        for(int j=0;j<3;j++)
        {
          FinalCOMTraj_deq[i].x[j] += deltaCOMTraj_deq_[i].x[j] ;
          FinalCOMTraj_deq[i].y[j] += deltaCOMTraj_deq_[i].y[j] ;
        }
      }
    }
    // Specify that we are in the ending phase.
    if (time <= m_SamplingPeriod )
      {
        if (EndingPhase_ == false)
        {
          TimeToStopOnLineMode_ = UpperTimeLimitToUpdate_ + SQP_T_ * SQP_N_ + m_SamplingPeriod;
        }
        UpperTimeLimitToUpdate_ = UpperTimeLimitToUpdate_ + SQP_T_ + m_SamplingPeriod ;
      }else{
        if (EndingPhase_ == false)
        {
          TimeToStopOnLineMode_ = UpperTimeLimitToUpdate_ + SQP_T_ * SQP_N_;
        }
        UpperTimeLimitToUpdate_ = UpperTimeLimitToUpdate_ + SQP_T_;
      }


  }
  //-----------------------------------
  //
  //
  //----------"Real-time" loop---------
}

void ZMPVelocityReferencedSQP::FullTrajectoryInterpolation(double time)
{
  std::vector<double> JerkX ;
  std::vector<double> JerkY ;
  std::vector<double> FootStepX ;
  std::vector<double> FootStepY ;
  std::vector<double> FootStepYaw ;
  NMPCgenerator_->getSolution(JerkX, JerkY, FootStepX, FootStepY, FootStepYaw);
  const std::deque<support_state_t> & SupportStates_deq = NMPCgenerator_->SupportStates_deq();
  LIPM_.setState(itCoM);
  LIPM_.Interpolation( COMTraj_deq_ctrl_, ZMPTraj_deq_ctrl_, CurrentIndex_,
                       JerkX[0], JerkY[0] );
  LIPM_.OneIteration( JerkX[0], JerkY[0] );
  itCoM = LIPM_.GetState();
  for ( int i = 1 ; i<previewSize_ ; i++ )
  {
    LIPM_.Interpolation( COMTraj_deq_ctrl_, ZMPTraj_deq_ctrl_, CurrentIndex_ + i * NbSampleControl_,
                         JerkX[i], JerkY[i] );
    LIPM_.OneIteration( JerkX[i], JerkY[i] );
  }

  OFTG_->interpolate_feet_positions(time, CurrentIndex_, SupportStates_deq,
                                    FootStepX, FootStepY, FootStepYaw,
                                    LeftFootTraj_deq_ctrl_, RightFootTraj_deq_ctrl_);

  unsigned int IndexMax = (int)round((previewDuration_+SQP_T_)  / InterpolationPeriod_ );
  ZMPTraj_deq_.resize(IndexMax);
  COMTraj_deq_.resize(IndexMax);
  LeftFootTraj_deq_.resize(IndexMax);
  RightFootTraj_deq_.resize(IndexMax);
  int inc =  (int)round(InterpolationPeriod_ / m_SamplingPeriod) ;
  for (unsigned int i = 0 , j = 0 ; j < IndexMax ; i = i + inc , ++j )
  {
    ZMPTraj_deq_[j] = ZMPTraj_deq_ctrl_[i] ;
    COMTraj_deq_[j] = COMTraj_deq_ctrl_[i] ;
    COMTraj_deq_[j].roll[0]  = 180/M_PI* COMTraj_deq_ctrl_[i].roll[0] ;
    COMTraj_deq_[j].pitch[0] = 180/M_PI* COMTraj_deq_ctrl_[i].pitch[0] ;
    COMTraj_deq_[j].yaw[0]   = 180/M_PI* COMTraj_deq_ctrl_[i].yaw[0] ;
    LeftFootTraj_deq_[j] = LeftFootTraj_deq_ctrl_[i] ;
    RightFootTraj_deq_[j] = RightFootTraj_deq_ctrl_[i] ;
  }

  Running_ = true ;
  if(SupportStates_deq[0].NbStepsLeft==0)
    Running_ = false ;
  return ;
}

// TODO: New parent class needed
void ZMPVelocityReferencedSQP::GetZMPDiscretization(deque<ZMPPosition> & ,
                                                   deque<COMState> & ,
                                                   deque<RelativeFootPosition> &,
                                                   deque<FootAbsolutePosition> &,
                                                   deque<FootAbsolutePosition> &,
                                                   double ,
                                                   COMState &,
                                                   MAL_S3_VECTOR(&,double),
                                                   FootAbsolutePosition & ,
                                                   FootAbsolutePosition & )
{
  cout << "To be removed" << endl;
}


void ZMPVelocityReferencedSQP::OnLineAddFoot(RelativeFootPosition & ,
                                            deque<ZMPPosition> & ,
                                            deque<COMState> & ,
                                            deque<FootAbsolutePosition> &,
                                            deque<FootAbsolutePosition> &,
                                            bool)
{
  cout << "To be removed" << endl;
}

int ZMPVelocityReferencedSQP::OnLineFootChange(double ,
                                              FootAbsolutePosition &,
                                              deque<ZMPPosition> & ,
                                              deque<COMState> & ,
                                              deque<FootAbsolutePosition> &,
                                              deque<FootAbsolutePosition> &,
                                              StepStackHandler  *)
{
  cout << "To be removed" << endl;
  return -1;
}

void ZMPVelocityReferencedSQP::EndPhaseOfTheWalking(deque<ZMPPosition> &,
                                                   deque<COMState> &,
                                                   deque<FootAbsolutePosition> &,
                                                   deque<FootAbsolutePosition> &)
{
  cout << "To be removed" << endl;
}

int ZMPVelocityReferencedSQP::ReturnOptimalTimeToRegenerateAStep()
{
  int r = (int)(m_PreviewControlTime/m_SamplingPeriod);
  return 2*r;
}
