#ifndef VECTOR_INCLUDED
#define VECTOR_INCLUDED

#include <boost/numeric/ublas/vector.hpp>

namespace OpenMps
{
	// 次元数
	const int DIM = 2;

	// 2次元ベクトル
	typedef boost::numeric::ublas::c_vector<double, DIM> Vector;
}
#endif