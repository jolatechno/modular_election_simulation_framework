#pragma once

#include "H5Cpp.h"


const H5::DataType &H5DataType(bool X) {
	return H5::PredType::NATIVE_HBOOL;
}
const H5::DataType &H5DataType(int X) {
	return H5::PredType::NATIVE_INT;
}
const H5::DataType &H5DataType(unsigned int X) {
	return H5::PredType::NATIVE_UINT;
}
const H5::DataType &H5DataType(long X) {
	return H5::PredType::NATIVE_LONG;
}
const H5::DataType &H5DataType(size_t X) {
	return H5::PredType::NATIVE_ULLONG;
}
const H5::DataType &H5DataType(float X) {
	return H5::PredType::NATIVE_FLOAT;
}
const H5::DataType &H5DataType(double X) {
	return H5::PredType::NATIVE_DOUBLE;
}
template<class Type>
const H5::DataType &H5DataType(Type X) {
static_assert(false, "Type not defined as serializable !");
	return H5::PredType::NATIVE_INT;
}

template<class Type>
void H5WriteVector(H5::Group &group, const std::vector<Type> &data, const char* data_name) {
	hsize_t dim[1] = { data.size() };
    H5::DataSpace dataspace = H5::DataSpace(1, dim);
    H5::DataSet   dataset   = group.createDataSet(data_name, H5DataType(data[0]), dataspace);

    dataset.write(&data[0], H5DataType(data[0]));
}

template<class Type>
void H5ReadVector(H5::Group &group, std::vector<Type> &data, const char* data_name) {
	H5::DataSet   dataset   = group.openDataSet(data_name);
	H5::DataSpace dataspace = dataset.getSpace();

    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims, NULL);

    data.resize(dims[0]);
    dataset.read(&data[0], H5DataType(data[0]), dataspace);
}