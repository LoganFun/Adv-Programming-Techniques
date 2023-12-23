// -----------------------------------------------------------------------//
// Author : Zilong Fan
// Class  : ECE 6122 A
// Date   : 9/25/2023
// Version: 1
// Description :
	//Implement all the functions need in point charge class 
// -----------------------------------------------------------------------//

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include "ECE_PointCharge.h"

// set the location of calculate point
void ECE_PointCharge::setLocation(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}
// set the charge
void ECE_PointCharge::setCharge(double q)
{
	this->q = q;
}
