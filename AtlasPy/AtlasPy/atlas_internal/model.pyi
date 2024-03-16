import atlas_internal.ast
import atlas_internal.core
from __future__ import annotations
import numpy
import typing
__all__ = ['LassoRegressionModelConfig', 'LinearRegressionModel', 'LinearRegressionModelConfig', 'LinearRegressionSolver', 'ModelBase', 'ModelConfig', 'ModelScalingType', 'ModelTarget', 'ModelTargetType', 'ModelType']
class LassoRegressionModelConfig(LinearRegressionModelConfig):
    alpha: float
    epsilon: float
    max_iter: int
    def __init__(self, base_config: ModelConfig, alpha: float = 1.0, epsilon: float = 0.0001, max_iter: int = 1000) -> None:
        ...
class LinearRegressionModel(ModelBase):
    def __init__(self, id: str, features: list[atlas_internal.ast.StrategyBufferOpNode], target: ModelTarget, config: LinearRegressionModelConfig) -> None:
        ...
    def getTheta(self) -> numpy.ndarray[numpy.float64[m, 1]]:
        ...
    def getX(self) -> numpy.ndarray[numpy.float64[m, n]]:
        ...
    def getY(self) -> numpy.ndarray[numpy.float64[m, 1]]:
        ...
class LinearRegressionModelConfig:
    fit_intercept: bool
    orthogonalize_features: bool
    def __init__(self, base_config: ModelConfig, solver: LinearRegressionSolver = ...) -> None:
        ...
class LinearRegressionSolver:
    """
    Members:
    
      LDLT
    
      ColPivHouseholderQR
    """
    ColPivHouseholderQR: typing.ClassVar[LinearRegressionSolver]  # value = <LinearRegressionSolver.ColPivHouseholderQR: 1>
    LDLT: typing.ClassVar[LinearRegressionSolver]  # value = <LinearRegressionSolver.LDLT: 0>
    __members__: typing.ClassVar[dict[str, LinearRegressionSolver]]  # value = {'LDLT': <LinearRegressionSolver.LDLT: 0>, 'ColPivHouseholderQR': <LinearRegressionSolver.ColPivHouseholderQR: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class ModelBase(atlas_internal.ast.StrategyBufferOpNode):
    pass
class ModelConfig:
    training_window: int
    type: ModelType
    walk_forward_window: int
    def __init__(self, training_window: int, walk_forward_window: int, model_type: ModelType, exchange: atlas_internal.core.Exchange) -> None:
        ...
class ModelScalingType:
    """
    Members:
    
      STANDARD
    
      MINMAX
    """
    MINMAX: typing.ClassVar[ModelScalingType]  # value = <ModelScalingType.MINMAX: 1>
    STANDARD: typing.ClassVar[ModelScalingType]  # value = <ModelScalingType.STANDARD: 0>
    __members__: typing.ClassVar[dict[str, ModelScalingType]]  # value = {'STANDARD': <ModelScalingType.STANDARD: 0>, 'MINMAX': <ModelScalingType.MINMAX: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class ModelTarget(atlas_internal.ast.StrategyBufferOpNode):
    def __init__(self, target: atlas_internal.ast.StrategyBufferOpNode, type: ModelTargetType, lookforward: int) -> None:
        ...
class ModelTargetType:
    """
    Members:
    
      ABSOLUTE
    
      DELTA
    
      PERCENTAGE_CHANGE
    """
    ABSOLUTE: typing.ClassVar[ModelTargetType]  # value = <ModelTargetType.ABSOLUTE: 0>
    DELTA: typing.ClassVar[ModelTargetType]  # value = <ModelTargetType.DELTA: 1>
    PERCENTAGE_CHANGE: typing.ClassVar[ModelTargetType]  # value = <ModelTargetType.PERCENTAGE_CHANGE: 2>
    __members__: typing.ClassVar[dict[str, ModelTargetType]]  # value = {'ABSOLUTE': <ModelTargetType.ABSOLUTE: 0>, 'DELTA': <ModelTargetType.DELTA: 1>, 'PERCENTAGE_CHANGE': <ModelTargetType.PERCENTAGE_CHANGE: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class ModelType:
    """
    Members:
    
      LINEAR_REGRESSION
    
      XGBOOST
    
      TORCH
    """
    LINEAR_REGRESSION: typing.ClassVar[ModelType]  # value = <ModelType.LINEAR_REGRESSION: 0>
    TORCH: typing.ClassVar[ModelType]  # value = <ModelType.TORCH: 2>
    XGBOOST: typing.ClassVar[ModelType]  # value = <ModelType.XGBOOST: 1>
    __members__: typing.ClassVar[dict[str, ModelType]]  # value = {'LINEAR_REGRESSION': <ModelType.LINEAR_REGRESSION: 0>, 'XGBOOST': <ModelType.XGBOOST: 1>, 'TORCH': <ModelType.TORCH: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
