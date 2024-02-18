module;
#include <Eigen/Dense>
export module AtlasLinAlg;

namespace Atlas
{

namespace LinAlg
{

export using EigenVectorXd = Eigen::VectorXd;
export using EigenMatrixXd = Eigen::MatrixXd;

export template<typename T>
using EigenMatrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

export template<typename T>
using EigenVector = Eigen::Matrix<T, Eigen::Dynamic, 1>;

export template<typename T>
using EigenMap = Eigen::Map<T>;

export template<typename T>
using EigenRef = Eigen::Ref<T>;

export template <typename T>
using EigenConstColView = Eigen::Block<Eigen::Matrix<T, -1, -1, 0, -1, -1>, -1, 1, true>;

export template <typename T>
using EigenConstRowView = Eigen::Block<Eigen::Matrix<T, -1, -1, 0, -1, -1>, 1, -1, false>;

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