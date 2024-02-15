// -----------------------------------------------------------------------//

// Version: 1
// Description :
	//define all the functions need in point charge class 
// -----------------------------------------------------------------------//

#pragma once
#ifndef _ECE_POINTCHARGE_
#define _ECE_POINTCHARGE_

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sstream>      // std::istringstream
#include <string>       // std::string

class ECE_PointCharge {

protected:

	double x;
	double y;
	double z;
	double q;

public:

	ECE_PointCharge() 
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
		q = 0.0;
	}

	// set the coordinates of 3D
	void setLocation(double x, double y, double z);

	// set the charge q
	void setCharge(double q);

};

#endif
