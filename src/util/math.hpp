#pragma once

#include <vector>
#include <algorithm>
#include <numeric>


namespace util::math {
	template<typename Type, typename limitType>
	std::vector<Type> linspace(const limitType &min, const limitType &max, const size_t N) {
		std::vector<Type> vect(N);

		const Type Tmin  = (Type)min;
		const Type delta = ((Type)max - Tmin)/N;
		for (size_t i = 0; i < N; ++i) {
			vect[i] = Tmin + delta*i;
		}

		return vect;
	}


	template<typename Type>
	Type integral(const std::vector<Type> &Y, const std::vector<Type> &X) {
		Type result = 0;
		for (size_t i = 0; i < Y.size()-1; ++i) {
			result += (Y[i+1] + Y[i])/2*(X[i+1] - X[i]);
		}
		return result;
	}

	template<typename Type>
	Type integral(const std::vector<Type> &Y, const Type &dX) {
		return std::accumulate(Y.begin(), Y.end(), (Type)0)*dX;
	}

	template<typename Type, class Matrix>
	std::vector<Type> integrals(const Matrix &Y, const std::vector<Type> &X) {
		std::vector<Type> results(Y.size(), 0);
		for (size_t i = 0; i < Y.size(); ++i) {
			for (size_t j = 0; i < X.size()-1; ++i) {
				results[i] += (Y[i][j+1] + Y[i][j])/2*(X[j+1] - X[j]);
			}
		}
		return results;
	}

	template<typename Type, class Matrix>
	std::vector<Type> integrals(const Matrix &Y, const Type &dX) {
		std::vector<Type> results(Y.size(), 0);
		for (size_t i = 0; i < Y.size(); ++i) {
			for (size_t j = 0; i < Y[i].size(); ++i) {
				results[i] += Y[i][j];
			}
			results[i] *= dX;
		}
		return results;
	}
}