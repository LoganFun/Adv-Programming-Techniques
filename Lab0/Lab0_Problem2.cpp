

#include<iostream>

using namespace std;

/*
*The goal is to find mulitples of 3 or 5, and sum the up.
*/
int main()
{
	int enter_Num = 1;
	int sum = 0; 
	int i, j, k;

	/*
	Do while Loop is used for the quit
	*/
	for(0;1;0)
	{	
		sum = 0;
		cout << "Please enter a natural number (0 to quit): ";
		cin >> enter_Num;
		if (enter_Num == 0)
			break;

		/*
		Goal is finding the multiples of 3
		with for loop
		*/
		cout << "The multiples of 3 below " << enter_Num << " are: ";
		for ( i = 3; (i+3) < enter_Num; i += 3)
		{
			cout << i << ",";
			sum += i; 
		}
		sum += i;
		cout << i << "." << endl;

		/*
		Goal is finding the multiples of 5
		with for loop
		*/
		cout << "The multiples of 5 below " << enter_Num << " are: ";
		for ( j = 5; (j+5) < enter_Num; j += 5)
		{
			cout << j << ",";
			sum += j;
		}
		sum += j;
		cout << j << "." << endl;

		/*
		Output the sum of all multiples
		with another for loop to find the repear values 
		and then delete them
		*/
		cout << "The sum of all multiples is: ";
		for ( k = 15; k < enter_Num; k += 15)
		{
			sum -= k;
		}
		cout << sum << "." << endl;

	} ;
	
	cout << "Program terminated" << endl;
	cout << "Have a nice day!" << endl;

	return 0;
}
