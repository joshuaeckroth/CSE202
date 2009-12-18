// adapted from /Engineering Problem Solving with C++/ by Etter & Ingber 2003
// "Program chapter2_7"

#include <iostream>
#include <climits>
#include <cfloat>

using namespace std;

int main()
{
	// Integer limits
	cout << "short max:\t\t\t" << SHRT_MAX << "\t\t\t(" << sizeof(short) << " bytes)" << endl;
	cout << "int max:\t\t\t" << INT_MAX << "\t\t(" << sizeof(int) << " bytes)" << endl;
	cout << "long max:\t\t\t" << LONG_MAX << "\t\t(" << sizeof(long) << " bytes)" << endl;
	cout << "long long max:\t\t\t" << LONG_LONG_MAX << "\t(" << sizeof(long long) << " bytes)" << endl << endl;

	// Float limits
	cout << "float max:\t\t\t" << FLT_MAX << "\t\t(" << sizeof(float) << " bytes)" << endl;
	cout << "float precision digits:\t\t" << FLT_DIG << endl;
	cout << "float max exponent:\t\t" << FLT_MAX_10_EXP << endl << endl;

	// Double limits
	cout << "double max:\t\t\t" << DBL_MAX << "\t\t(" << sizeof(double) << " bytes)" << endl;
	cout << "double precision digits:\t" << DBL_DIG << endl;
	cout << "double maximum exponent:\t" << DBL_MAX_10_EXP << endl << endl;

	// Long double limits
	cout << "long double max:\t\t" << LDBL_MAX << "\t\t\t(" << sizeof(long double) << " bytes)" << endl;
	cout << "long double precision digits:\t" << LDBL_DIG << endl;
	cout << "long double maximum exponent:\t" << LDBL_MAX_10_EXP << endl << endl;

	return EXIT_SUCCESS;
}



