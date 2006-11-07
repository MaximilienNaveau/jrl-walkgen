/* Inverse Kinematics for legs and arms of a canonical
   humanoid robot. The arm are supposed to have 2 links.
   The legs are supposed to have 3 links.
   Please look at the documentation for more information.

   CVS Information:
   $Id: InverseKinematics.cpp,v 1.2 2006-01-18 06:34:58 stasse Exp $
   $Author: stasse $
   $Date: 2006-01-18 06:34:58 $
   $Revision: 1.2 $
   $Source: /home/CVSREPOSITORY/PatternGeneratorJRL/src/InverseKinematics.cpp,v $
   $Log: InverseKinematics.cpp,v $
   Revision 1.2  2006-01-18 06:34:58  stasse
   OS: Updated the names of the contributors, the documentation
   and added a sample file for WalkPlugin



   Copyright (c) 2005-2006, 
   Olivier Stasse,
   Ramzi Sellouati
   
   JRL-Japan, CNRS/AIST

   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without modification, 
   are permitted provided that the following conditions are met:
   
   * Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
   * Neither the name of the <ORGANIZATION> nor the names of its contributors 
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
#include <math.h>
#include <InverseKinematics.h>


using namespace::PatternGeneratorJRL;

InverseKinematics::InverseKinematics()
{
m_KneeAngleBound=0.0*M_PI/180.0;
m_KneeAngleBoundCos=cos(m_KneeAngleBound);
m_KneeAngleBound1=30.0*M_PI/180.0; //sets a minimum angle for the knee and protects for overstretch
m_KneeAngleBoundCos1=cos(m_KneeAngleBound1);  //used during inverse kin calculations

//m_KneeAngleBound=15.0*M_PI/180.0; //sets a minimum angle for the knee and protects for overstretch
m_KneeAngleBoundCos2= 1.336;//cos(m_KneeAngleBound2);
}

InverseKinematics::~InverseKinematics()
{
}

int InverseKinematics::ComputeInverseKinematicsForLegs3(VNL::Matrix<double> Body_R,
						       VNL::Matrix<double> Body_P,
						       VNL::Matrix<double> Dt,
						       VNL::Matrix<double> Foot_R,
						       VNL::Matrix<double> Foot_P,
						       VNL::Matrix<double> &q)
{
  double A=0.3,B=0.3,C,c5,q6a;
  VNL::Matrix<double> r(3,1);
  VNL::Matrix<double> rT(3,3);
#if 0
  cout << "Body_R" << Body_R<<endl;
  cout << "Body_P" << Body_P<<endl;

  cout << "Foot_R" << Foot_R<<endl;
  cout << "Foot_P" << Foot_P<<endl;
  
  cout << "Dt" << Dt << endl;
#endif
  rT= Foot_R.Transpose();
#if 0
  cout << "rT" << rT<< endl;
  cout << "rT" << rT<< endl;
  cout << "Body_R * Dt - Foot_P" << Body_R * Dt - Foot_P << endl;
#endif
  r = rT * (Body_P +  Body_R * Dt - Foot_P);
  C = sqrt(r(0,0)*r(0,0)+
	   r(1,0)*r(1,0)+
	   r(2,0)*r(2,0));
  //C2 =sqrt(C1*C1-D*D);
  c5 = (C*C-A*A-B*B)/(2.0*A*B);
  //cout << r(0,0) << " " << r(1,0) << " "  << r(2,0) <<" ";
  //  cout << "C " << C << " c5 " <<c5 <<endl;
  //  cout << C << " " ;
  if (c5>=1)
    {
      q(3,0)= 0.0;
    }
  else if (c5<=-1.0)
    {
      q(3,0)= M_PI;
    }
  else 
    {
      q(3,0)= acos(c5);
    }
  q6a = asin((A/C)*sin(M_PI- q(3,0)));
  // q6b = atan2(D,C2);


  float c,s,cz,sz;

  q(5,0) = atan2(r(1,0),r(2,0));
  if (q(5,0)>M_PI/2.0)
    {
      q(5,0) = q(5,0)-M_PI;
    }
  else if (q(5,0)<-M_PI/2.0)
    {
      q(5,0)+= M_PI;
    }

  q(4,0) = -atan2(r(0,0), (r(2,0)<0? -1.0:1.0)*sqrt(r(1,0)*r(1,0)+r(2,0)*r(2,0) )) - q6a;

  VNL::Matrix<double> R(3,3),BRt(3,3);

  BRt = Body_R.Transpose();

  VNL::Matrix<double> Rroll(3,3);
  c = cos(-q(5,0));
  s = sin(-q(5,0));

  Rroll(0,0) = 1.0;   Rroll(0,1) = 0.0;   Rroll(0,2) = 0.0; 
  Rroll(1,0) = 0.0;   Rroll(1,1) = c;   Rroll(1,2) = -s; 
  Rroll(2,0) = 0.0;   Rroll(2,1) = s;   Rroll(2,2) = c; 


  VNL::Matrix<double> Rpitch(3,3);
  c = cos(-q(4,0)-q(3,0));
  s = sin(-q(4,0)-q(3,0));

  Rpitch(0,0) = c;     Rpitch(0,1) = 0;   Rpitch(0,2) = s; 
  Rpitch(1,0) = 0.0;   Rpitch(1,1) = 1;   Rpitch(1,2) = 0; 
  Rpitch(2,0) = -s;    Rpitch(2,1) = 0;   Rpitch(2,2) = c; 

  //  cout << " BRt"  << BRt << endl;
  R = BRt * Foot_R * Rroll *Rpitch;
  q(0,0) = atan2(-R(0,1),R(1,1));
  
  cz = cos(q(0,0)); sz = sin(q(0,0));
  q(1,0) = atan2(R(2,1), -R(0,1)*sz + R(1,1) *cz);
  q(2,0) = atan2( -R(2,0), R(2,2));

  //  exit(0);
  return 0;
}

int InverseKinematics::ComputeInverseKinematics2ForLegs(VNL::Matrix<double> Body_R,
							VNL::Matrix<double> Body_P,
							VNL::Matrix<double> Dt,
							VNL::Matrix<double> Foot_R,
							VNL::Matrix<double> Foot_P,
							VNL::Matrix<double> &q)
{
  double A=0.3,B=0.3,C,c5,q6a;
  VNL::Matrix<double> r(3,1);
  VNL::Matrix<double> rT(3,3);
  VNL::Matrix<double> Foot_Rt(3,3);
  double NormofDt;

  // New part for the inverse kinematics specific to the HRP-2
  // robot. The computation of rx, ry and rz is different.

  // We compute the position of the body inside the reference
  // frame of the foot.
  VNL::Matrix<double> v(3,1);
  double theta, psi, Cp;
  float OppSignOfDtY = Dt(1,0) < 0.0 ? 1.0 : -1.0;

  Foot_Rt = Foot_R.Transpose();
  v = Body_P - Foot_P;
  v = Foot_Rt * v;
  //  cout << "v : "<< v <<endl;
  r(0,0) = v(0,0);
  NormofDt = sqrt(Dt(0,0)*Dt(0,0) + Dt(1,0)*Dt(1,0) + Dt(2,0)*Dt(2,0));
  //  cout << "Norm of Dt: " << NormofDt << endl;
  Cp = sqrt(v(1,0)*v(1,0)+v(2,0)*v(2,0) - NormofDt * NormofDt);
  psi = OppSignOfDtY * atan2(NormofDt,Cp);

  
  //  cout << "vz: " << v(2,0) << " vy :" << v(1,0) << endl;
#if 0
  if (v(1,0)<0.0)
    {
      theta = atan2(v(2,0),-v(1,0));
      r(1,0) = -cos(psi+theta)*Cp;
    }
  else 
    {
      theta = atan2(v(2,0),v(1,0));
      r(1,0) = cos(psi+theta)*Cp;
    }
#else
  theta = atan2(v(2,0),v(1,0));
  
  r(1,0) = cos(psi+theta)*Cp;
#endif

  r(2,0) = sin(psi+theta)*Cp;
  
  //  cout << "r : " << r << endl;
#if 0
  cout << "Body_R" << Body_R<<endl;
  cout << "Body_P" << Body_P<<endl;

  cout << "Foot_R" << Foot_R<<endl;
  cout << "Foot_P" << Foot_P<<endl;
  
  cout << "Dt" << Dt << endl;
#endif

#if 0
  cout << "rT" << rT<< endl;
  cout << "rT" << rT<< endl;
  cout << "Body_R * Dt - Foot_P" << Body_R * Dt - Foot_P << endl;
#endif
  //  r = rT * (Body_P +  Body_R * Dt - Foot_P);
  C = sqrt(r(0,0)*r(0,0)+
	   r(1,0)*r(1,0)+
	   r(2,0)*r(2,0));
  //C2 =sqrt(C1*C1-D*D);
  c5 = (C*C-A*A-B*B)/(2.0*A*B);
  //cout << r(0,0) << " " << r(1,0) << " "  << r(2,0) <<" ";
  //  cout << "C " << C << " c5 " <<c5 <<endl;
  //  cout << C << " " ;
  if (c5>=m_KneeAngleBoundCos)
    {
	//double klojo;

	q(3,0)=m_KneeAngleBound;

/*	if (c5>=m_KneeAngleBoundCos2)
		q(3,0) =m_KneeAngleBound;
        else 
	{
 
  

double a,b,c,d;
	a =  6.59620337503859;
	b = -13.77121203051706;
	c = 9.82223457054312;
	d = -2.32950990645471;

	q(3,0)= a+b*c5+c*c5*c5+d*c5*c5*c5;

	}
		

	
*/    
	}
  else if (c5<=-1.0)
    {
      q(3,0)= M_PI;
    }
  else 
    {
     q(3,0)= acos(c5);
    }
  q6a = asin((A/C)*sin(M_PI- q(3,0)));
  // q6b = atan2(D,C2);


  float c,s,cz,sz;

  q(5,0) = atan2(r(1,0),r(2,0));
  if (q(5,0)>M_PI/2.0)
    {
      q(5,0) = q(5,0)-M_PI;
    }
  else if (q(5,0)<-M_PI/2.0)
    {
      q(5,0)+= M_PI;
    }

  q(4,0) = -atan2(r(0,0), (r(2,0)<0? -1.0:1.0)*sqrt(r(1,0)*r(1,0)+r(2,0)*r(2,0) )) - q6a;

  VNL::Matrix<double> R(3,3),BRt(3,3);

  BRt = Body_R.Transpose();

  VNL::Matrix<double> Rroll(3,3);
  c = cos(-q(5,0));
  s = sin(-q(5,0));

  Rroll(0,0) = 1.0;   Rroll(0,1) = 0.0;   Rroll(0,2) = 0.0; 
  Rroll(1,0) = 0.0;   Rroll(1,1) = c;   Rroll(1,2) = -s; 
  Rroll(2,0) = 0.0;   Rroll(2,1) = s;   Rroll(2,2) = c; 


  VNL::Matrix<double> Rpitch(3,3);
  c = cos(-q(4,0)-q(3,0));
  s = sin(-q(4,0)-q(3,0));

  Rpitch(0,0) = c;     Rpitch(0,1) = 0;   Rpitch(0,2) = s; 
  Rpitch(1,0) = 0.0;   Rpitch(1,1) = 1;   Rpitch(1,2) = 0; 
  Rpitch(2,0) = -s;    Rpitch(2,1) = 0;   Rpitch(2,2) = c; 

  //  cout << " BRt"  << BRt << endl;
  R = BRt * Foot_R * Rroll *Rpitch;
  q(0,0) = atan2(-R(0,1),R(1,1));
  
  cz = cos(q(0,0)); sz = sin(q(0,0));
  q(1,0) = atan2(R(2,1), -R(0,1)*sz + R(1,1) *cz);
  q(2,0) = atan2( -R(2,0), R(2,2));

  //  exit(0);
  return 0;
}

double InverseKinematics::ComputeXmax(double & Z)
{
  double A=0.25,
    B=0.25;
  double Xmax;
  if (Z<0.0)
    Z = 2*A*cos(15*M_PI/180.0);
  Xmax = sqrt(A*A - (Z - B)*(Z-B));
  return Xmax;
}

int InverseKinematics::ComputeInverseKinematicsForArms(double X,double Z,
						       double &Alpha,
						       double &Beta)
{
  double A=0.25,
    B=0.25;

  double C=0.0,Gamma=0.0,Theta=0.0;
  C = sqrt(X*X+Z*Z);
  
  Beta = acos((A*A+B*B-C*C)/(2*A*B))- M_PI;
  Gamma = asin((B*sin(M_PI+Beta))/C);
  Theta = atan2(X,Z);
  Alpha = Gamma - Theta;
  return 0;
}
