// -----------------------------------------------------------------------//
// Author : Zilong Fan
// Class  : ECE 6122 A
// Date   : 9/25/2023
// Version: 1
// Description :
	//Define all the functions need in electric field class 
// -----------------------------------------------------------------------//

#pragma once
#ifndef _ECE_ElectricField_
#define _ECE_ElectricField_
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include "ECE_PointCharge.h"

class ECE_ElectricField : public ECE_PointCharge
{
protected:
	double Ex;
	double Ey;
	double Ez;
	double E;
	double x_Pos;
	double y_Pos;
	double z_Pos;
	double k_micro = 9000.0;
	

public:
	int exe_signal = 0;

	// constructor
	ECE_ElectricField()
	{
		ECE_PointCharge();
		Ex = Ey = Ez = E = x_Pos = y_Pos = z_Pos = 0,0;
		x = y = z = q = 0.0;
		exe_signal = 0;
	}
	
	void At(double a, double b, double c);
	
	void Zero();
	
	void computeFieldAt(double a, double b, double c);

	void find_sum(ECE_ElectricField E_sub);
	double find_x();
	double find_y();
	double find_z();
	double find_Ex();
	double find_Ey();
	double find_Ez();
	
	void exe();

	void getElectricField(double& Ex, double& Ey, double& Ez, double& E);
	
	void getElectricField_self();
};

#endif


