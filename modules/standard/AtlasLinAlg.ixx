module;
#include <Eigen/Dense>
export module AtlasLinAlg;

namespace Atlas
{

namespace LinAlg
{

export using EigenVectorXd = Eigen::VectorXd;

export template <typename T>
using EigenConstColView = Eigen::Block<Eigen::Matrix<T, -1, -1, 0, -1, -1>, -1, 1, true>;

export template <typename T>
using EigenBlockView = Eigen::Block<Eigen::Matrix<T, -1, -1, 0, -1, -1>, -1, -1, true>;

export using EigenCwiseProductOp = Eigen::internal::scalar_product_op<double, double>;
export using EigenCwiseQuotientOp = Eigen::internal::scalar_quotient_op<double, double>;
export using EigenCwiseSumOp = Eigen::internal::scalar_sum_op<double, double>;
export using EigenCwiseDifferenceOp = Eigen::internal::scalar_difference_op<double, double>;

export template <typename EigenCwiseBinOpType>
using EigenCwiseBinOp =
const Eigen::CwiseBinaryOp<EigenCwiseBinOpType, const EigenConstColView<double>, const EigenConstColView<double>>;
}

}