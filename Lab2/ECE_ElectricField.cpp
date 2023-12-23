// -----------------------------------------------------------------------//
// Author : Zilong Fan
// Class  : ECE 6122 A
// Date   : 9/25/2023
// Version: 1
// Description :
	//Implement all the functions need in electric field class 
// -----------------------------------------------------------------------//

#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include <cmath>
#include "ECE_ElectricField.h"


void ECE_ElectricField::At(double a, double b, double c) 
{
	x_Pos = a;
	y_Pos = b;
	z_Pos = c;
}

void ECE_ElectricField::Zero()
{
	Ex = Ey = Ez = E = x_Pos = y_Pos = z_Pos = 0, 0;
	x = y = z = q = 0.0;
	exe_signal = 0;
}

double ECE_ElectricField::find_x()
{
	return x;
}

double ECE_ElectricField::find_y()
{
	return y;
}

double ECE_ElectricField::find_z()
{
	return z;
}

double ECE_ElectricField::find_Ex()
{
	return Ex;
}

double ECE_ElectricField::find_Ey()
{
	return Ey;
}

double ECE_ElectricField::find_Ez()
{
	return Ez;
}

void ECE_ElectricField::exe()
{
	if(exe_signal==0)
		exe_signal = 1;
	// test repeat calculation
	else
		std::cout << "wrong!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
}

void ECE_ElectricField::computeFieldAt(double a, double b, double c)
{
	 double r = sqrt((c - z) * (c - z) +
		 (a - x) * (a - x) + (b - y) * (b - y));

	 Ex = k_micro * q / r / r * (a - x) / r;

	 Ey = k_micro * q / r / r * (b - y) / r;

	 Ez = k_micro * q / r / r * (c - z) / r;
}

void ECE_ElectricField::find_sum(ECE_ElectricField E_sub) 
{
	Ex += E_sub.find_Ex();
	Ey += E_sub.find_Ey();
	Ez += E_sub.find_Ez();
}

void ECE_ElectricField::getElectricField(double& Ex, double& Ey, double& Ez, double &E)
{
	std::cout << "The electric field at (" << x_Pos << ", " << y_Pos << ", " << z_Pos << ") in V/m is" << std::endl;
	std::cout << std::scientific;
	std::cout << "Ex = " << Ex << std::endl;
	std::cout << "Ey = " << Ey << std::endl;
	std::cout << "Ez = " << Ez << std::endl;
	E = sqrt(Ex * Ex + Ey * Ey + Ez * Ez);
	std::cout << "|E| = " << E << std::endl;
}

// obtain the electric field 
void ECE_ElectricField::getElectricField_self()
{
	std::cout << "The electric field at (" << x_Pos << ", " << y_Pos << ", " << z_Pos << ") in V/m is" << std::endl;
	std::cout << std::scientific;
	std::cout << std::setprecision(4) << "Ex = " << this->Ex << std::endl;
	std::cout << std::setprecision(4) << "Ey = " << this->Ey << std::endl;
	std::cout << std::setprecision(4) << "Ez = " << this->Ez << std::endl;
	E = sqrt(Ex * Ex + Ey * Ey + Ez * Ez);
	std::cout << std::setprecision(4) << "|E| = " << E << std::endl;
}

