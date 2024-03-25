#pragma once
#include <Eigen/Dense>

namespace Atlas
{

namespace LinAlg
{

 using EigenVectorXd = Eigen::VectorXd;
 using EigenMatrixXd = Eigen::MatrixXd;
 using EigenVectorXf = Eigen::VectorXf;
 using EigenMatrixXf = Eigen::MatrixXf;
 using EigenVectorXi = Eigen::VectorXi;

 template<typename T>
using EigenMatrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

 template<typename T>
using EigenVector = Eigen::Matrix<T, Eigen::Dynamic, 1>;

 using EigenBlock = const Eigen::Block<const Eigen::Matrix<double, -1, -1, 0, -1, -1>, -1, -1, false>;

 template<typename T>
using EigenMap = Eigen::Map<T>;

 template<typename T>
using EigenRef = Eigen::Ref<T>;

 template <typename T>
using EigenConstColView = Eigen::Block<Eigen::Matrix<T, -1, -1, 0, -1, -1>, -1, 1, true>;

 template <typename T>
using EigenConstRowView = Eigen::Block<Eigen::Matrix<T, -1, -1, 0, -1, -1>, 1, -1, false>;

 template <typename T>
using EigenBlockView = Eigen::Block<Eigen::Matrix<T, -1, -1, 0, -1, -1>, -1, -1, true>;

 using EigenCwiseProductOp = Eigen::internal::scalar_product_op<double, double>;
 using EigenCwiseQuotientOp = Eigen::internal::scalar_quotient_op<double, double>;
 using EigenCwiseSumOp = Eigen::internal::scalar_sum_op<double, double>;
 using EigenCwiseDifferenceOp = Eigen::internal::scalar_difference_op<double, double>;

 template <typename EigenCwiseBinOpType>
using EigenCwiseBinOp =
const Eigen::CwiseBinaryOp<EigenCwiseBinOpType, const EigenConstColView<double>, const EigenConstColView<double>>;
}

}