import atlas_internal.core
from __future__ import annotations
import numpy
import typing
__all__ = ['ABS', 'ADD', 'AND', 'ASTNode', 'ATRNode', 'AllocationBaseNode', 'AllocationNode', 'AllocationType', 'AllocationWeightNode', 'AssetCompNode', 'AssetCompType', 'AssetFunctionNode', 'AssetFunctionType', 'AssetIfNode', 'AssetMedianNode', 'AssetObserverNode', 'AssetOpNode', 'AssetOpType', 'AssetReadNode', 'AssetScalerNode', 'CONDITIONAL_SPLIT', 'CovarianceNode', 'CovarianceNodeBase', 'CovarianceObserverNode', 'CovarianceType', 'DIVIDE', 'DummyNode', 'EQUAL', 'EVRankNode', 'EVRankType', 'ExchangeViewFilter', 'ExchangeViewFilterType', 'ExchangeViewNode', 'FULL', 'FixedAllocationNode', 'GREATER', 'GREATER_EQUAL', 'GREATER_THAN', 'GridDimension', 'GridDimensionLimit', 'GridDimensionObserver', 'GridType', 'INCREMENTAL', 'IncrementalCovarianceNode', 'InvVolWeight', 'LESS', 'LESS_EQUAL', 'LESS_THAN', 'LOG', 'LOWER_TRIANGULAR', 'LagNode', 'LogicalType', 'MULTIPLY', 'MaxObserverNode', 'MeanObserverNode', 'NEXTREME', 'NLARGEST', 'NLV', 'NOT_EQUAL', 'NSMALLEST', 'OR', 'ORDERS_EAGER', 'POWER', 'PeriodicTriggerNode', 'SIGN', 'STOP_LOSS', 'SUBTRACT', 'StrategyBufferOpNode', 'StrategyGrid', 'StrategyMonthlyRunnerNode', 'StrategyNode', 'SumObserverNode', 'TAKE_PROFIT', 'Tracer', 'TracerType', 'TradeLimitNode', 'TradeLimitType', 'TriggerNode', 'TsArgMaxObserverNode', 'UNIFORM', 'UPPER_TRIANGULAR', 'VOLATILITY', 'VarianceObserverNode', 'WEIGHTS']
class ASTNode:
    pass
class ATRNode(StrategyBufferOpNode):
    @staticmethod
    def make(arg0: atlas_internal.core.Exchange, arg1: str, arg2: str, arg3: int) -> ATRNode:
        ...
class AllocationBaseNode:
    def getTradeLimitNode(self) -> TradeLimitNode | None:
        ...
    def setTradeLimit(self, arg0: TradeLimitType, arg1: float) -> None:
        ...
    def setWeightScale(self, arg0: ...) -> None:
        ...
class AllocationNode(AllocationBaseNode):
    @staticmethod
    def make(exchange_view: StrategyBufferOpNode, type: AllocationType = ..., alloc_param: float | None = None, epsilon: float = 0.0) -> AllocationNode:
        ...
class AllocationType:
    """
    Members:
    
      UNIFORM
    
      CONDITIONAL_SPLIT
    """
    CONDITIONAL_SPLIT: typing.ClassVar[AllocationType]  # value = <AllocationType.CONDITIONAL_SPLIT: 1>
    UNIFORM: typing.ClassVar[AllocationType]  # value = <AllocationType.UNIFORM: 0>
    __members__: typing.ClassVar[dict[str, AllocationType]]  # value = {'UNIFORM': <AllocationType.UNIFORM: 0>, 'CONDITIONAL_SPLIT': <AllocationType.CONDITIONAL_SPLIT: 1>}
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
class AllocationWeightNode(StrategyBufferOpNode):
    pass
class AssetCompNode(StrategyBufferOpNode):
    def __init__(self, arg0: StrategyBufferOpNode, arg1: LogicalType, arg2: StrategyBufferOpNode, arg3: StrategyBufferOpNode, arg4: StrategyBufferOpNode) -> None:
        ...
    def swapFalseEval(self, arg0: StrategyBufferOpNode) -> None:
        ...
    def swapTrueEval(self, arg0: StrategyBufferOpNode) -> None:
        ...
class AssetCompType:
    """
    Members:
    
      EQUAL
    
      NOT_EQUAL
    
      GREATER
    
      LESS
    
      GREATER_EQUAL
    
      LESS_EQUAL
    """
    EQUAL: typing.ClassVar[AssetCompType]  # value = <AssetCompType.EQUAL: 0>
    GREATER: typing.ClassVar[AssetCompType]  # value = <AssetCompType.GREATER: 2>
    GREATER_EQUAL: typing.ClassVar[AssetCompType]  # value = <AssetCompType.GREATER_EQUAL: 4>
    LESS: typing.ClassVar[AssetCompType]  # value = <AssetCompType.LESS: 3>
    LESS_EQUAL: typing.ClassVar[AssetCompType]  # value = <AssetCompType.LESS_EQUAL: 5>
    NOT_EQUAL: typing.ClassVar[AssetCompType]  # value = <AssetCompType.NOT_EQUAL: 1>
    __members__: typing.ClassVar[dict[str, AssetCompType]]  # value = {'EQUAL': <AssetCompType.EQUAL: 0>, 'NOT_EQUAL': <AssetCompType.NOT_EQUAL: 1>, 'GREATER': <AssetCompType.GREATER: 2>, 'LESS': <AssetCompType.LESS: 3>, 'GREATER_EQUAL': <AssetCompType.GREATER_EQUAL: 4>, 'LESS_EQUAL': <AssetCompType.LESS_EQUAL: 5>}
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
class AssetFunctionNode(StrategyBufferOpNode):
    def __init__(self, arg0: StrategyBufferOpNode, arg1: AssetFunctionType, arg2: float | None) -> None:
        ...
class AssetFunctionType:
    """
    Members:
    
      SIGN
    
      POWER
    
      ABS
    
      LOG
    """
    ABS: typing.ClassVar[AssetFunctionType]  # value = <AssetFunctionType.ABS: 3>
    LOG: typing.ClassVar[AssetFunctionType]  # value = <AssetFunctionType.LOG: 4>
    POWER: typing.ClassVar[AssetFunctionType]  # value = <AssetFunctionType.POWER: 1>
    SIGN: typing.ClassVar[AssetFunctionType]  # value = <AssetFunctionType.SIGN: 0>
    __members__: typing.ClassVar[dict[str, AssetFunctionType]]  # value = {'SIGN': <AssetFunctionType.SIGN: 0>, 'POWER': <AssetFunctionType.POWER: 1>, 'ABS': <AssetFunctionType.ABS: 3>, 'LOG': <AssetFunctionType.LOG: 4>}
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
class AssetIfNode(StrategyBufferOpNode):
    def __init__(self, arg0: StrategyBufferOpNode, arg1: AssetCompType, arg2: StrategyBufferOpNode) -> None:
        ...
    def swapLeftEval(self, arg0: StrategyBufferOpNode) -> None:
        ...
    def swapRightEval(self, arg0: StrategyBufferOpNode) -> None:
        ...
class AssetMedianNode(StrategyBufferOpNode):
    @staticmethod
    def make(arg0: atlas_internal.core.Exchange, arg1: str, arg2: str) -> AssetMedianNode:
        ...
class AssetObserverNode(StrategyBufferOpNode):
    pass
class AssetOpNode(StrategyBufferOpNode):
    @staticmethod
    def make(arg0: StrategyBufferOpNode, arg1: StrategyBufferOpNode, arg2: AssetOpType) -> AssetOpNode:
        ...
    def getSwapLeft(self) -> int:
        ...
    def getSwapRight(self) -> int:
        ...
class AssetOpType:
    """
    Members:
    
      ADD
    
      SUBTRACT
    
      MULTIPLY
    
      DIVIDE
    """
    ADD: typing.ClassVar[AssetOpType]  # value = <AssetOpType.ADD: 0>
    DIVIDE: typing.ClassVar[AssetOpType]  # value = <AssetOpType.DIVIDE: 3>
    MULTIPLY: typing.ClassVar[AssetOpType]  # value = <AssetOpType.MULTIPLY: 2>
    SUBTRACT: typing.ClassVar[AssetOpType]  # value = <AssetOpType.SUBTRACT: 1>
    __members__: typing.ClassVar[dict[str, AssetOpType]]  # value = {'ADD': <AssetOpType.ADD: 0>, 'SUBTRACT': <AssetOpType.SUBTRACT: 1>, 'MULTIPLY': <AssetOpType.MULTIPLY: 2>, 'DIVIDE': <AssetOpType.DIVIDE: 3>}
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
class AssetReadNode(StrategyBufferOpNode):
    @staticmethod
    def make(arg0: str, arg1: int, arg2: atlas_internal.core.Exchange) -> AssetReadNode:
        ...
class AssetScalerNode(StrategyBufferOpNode):
    def __init__(self, arg0: StrategyBufferOpNode, arg1: AssetOpType, arg2: float) -> None:
        ...
class CovarianceNode(CovarianceNodeBase):
    pass
class CovarianceNodeBase:
    def getCovarianceMatrix(self) -> numpy.ndarray[numpy.float64[m, n]]:
        ...
class CovarianceObserverNode(AssetObserverNode):
    def __init__(self, arg0: str, arg1: StrategyBufferOpNode, arg2: StrategyBufferOpNode, arg3: int) -> None:
        ...
class CovarianceType:
    """
    Members:
    
      FULL
    
      INCREMENTAL
    """
    FULL: typing.ClassVar[CovarianceType]  # value = <CovarianceType.FULL: 0>
    INCREMENTAL: typing.ClassVar[CovarianceType]  # value = <CovarianceType.INCREMENTAL: 1>
    __members__: typing.ClassVar[dict[str, CovarianceType]]  # value = {'FULL': <CovarianceType.FULL: 0>, 'INCREMENTAL': <CovarianceType.INCREMENTAL: 1>}
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
class DummyNode(StrategyBufferOpNode):
    def __init__(self, arg0: atlas_internal.core.Exchange) -> None:
        ...
class EVRankNode(StrategyBufferOpNode):
    @staticmethod
    def make(ev: ExchangeViewNode, type: EVRankType, count: int) -> EVRankNode:
        ...
class EVRankType:
    """
    Members:
    
      NLARGEST
    
      NSMALLEST
    
      NEXTREME
    """
    NEXTREME: typing.ClassVar[EVRankType]  # value = <EVRankType.NEXTREME: 2>
    NLARGEST: typing.ClassVar[EVRankType]  # value = <EVRankType.NLARGEST: 0>
    NSMALLEST: typing.ClassVar[EVRankType]  # value = <EVRankType.NSMALLEST: 1>
    __members__: typing.ClassVar[dict[str, EVRankType]]  # value = {'NLARGEST': <EVRankType.NLARGEST: 0>, 'NSMALLEST': <EVRankType.NSMALLEST: 1>, 'NEXTREME': <EVRankType.NEXTREME: 2>}
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
class ExchangeViewFilter:
    def __init__(self, arg0: ExchangeViewFilterType, arg1: float, arg2: float | None) -> None:
        ...
class ExchangeViewFilterType:
    """
    Members:
    
      GREATER_THAN
    
      LESS_THAN
    """
    GREATER_THAN: typing.ClassVar[ExchangeViewFilterType]  # value = <ExchangeViewFilterType.GREATER_THAN: 0>
    LESS_THAN: typing.ClassVar[ExchangeViewFilterType]  # value = <ExchangeViewFilterType.LESS_THAN: 1>
    __members__: typing.ClassVar[dict[str, ExchangeViewFilterType]]  # value = {'GREATER_THAN': <ExchangeViewFilterType.GREATER_THAN: 0>, 'LESS_THAN': <ExchangeViewFilterType.LESS_THAN: 1>}
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
class ExchangeViewNode(StrategyBufferOpNode):
    @staticmethod
    def make(exchange: atlas_internal.core.Exchange, asset_op_node: StrategyBufferOpNode, filter: ExchangeViewFilter | None = None, left_view: ExchangeViewNode | None = None) -> ExchangeViewNode:
        ...
class FixedAllocationNode(AllocationBaseNode):
    @staticmethod
    def make(arg0: list[tuple[str, float]], arg1: atlas_internal.core.Exchange, arg2: float) -> FixedAllocationNode:
        ...
class GridDimension:
    pass
class GridDimensionLimit(GridDimension):
    @staticmethod
    def make(name: str, dimension_values: list[float], node: TradeLimitNode, getter: int, setter: int) -> GridDimensionLimit:
        ...
class GridDimensionObserver(GridDimension):
    @staticmethod
    def make(name: str, dimension_values: list[float], observer_base: AssetObserverNode, observer_child: StrategyBufferOpNode, swap_addr: int) -> GridDimensionObserver:
        ...
class GridType:
    """
    Members:
    
      UPPER_TRIANGULAR
    
      LOWER_TRIANGULAR
    """
    LOWER_TRIANGULAR: typing.ClassVar[GridType]  # value = <GridType.LOWER_TRIANGULAR: 2>
    UPPER_TRIANGULAR: typing.ClassVar[GridType]  # value = <GridType.UPPER_TRIANGULAR: 1>
    __members__: typing.ClassVar[dict[str, GridType]]  # value = {'UPPER_TRIANGULAR': <GridType.UPPER_TRIANGULAR: 1>, 'LOWER_TRIANGULAR': <GridType.LOWER_TRIANGULAR: 2>}
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
class IncrementalCovarianceNode(CovarianceNodeBase):
    pass
class InvVolWeight(AllocationWeightNode):
    def __init__(self, arg0: CovarianceNodeBase, arg1: float | None) -> None:
        ...
class LagNode(StrategyBufferOpNode):
    pass
class LogicalType:
    """
    Members:
    
      AND
    
      OR
    """
    AND: typing.ClassVar[LogicalType]  # value = <LogicalType.AND: 0>
    OR: typing.ClassVar[LogicalType]  # value = <LogicalType.OR: 1>
    __members__: typing.ClassVar[dict[str, LogicalType]]  # value = {'AND': <LogicalType.AND: 0>, 'OR': <LogicalType.OR: 1>}
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
class MaxObserverNode(AssetObserverNode):
    def __init__(self, arg0: str, arg1: StrategyBufferOpNode, arg2: int) -> None:
        ...
class MeanObserverNode(AssetObserverNode):
    def __init__(self, arg0: str, arg1: StrategyBufferOpNode, arg2: int) -> None:
        ...
class PeriodicTriggerNode(TriggerNode):
    @staticmethod
    def make(exchange: atlas_internal.core.Exchange, frequency: int) -> TriggerNode:
        ...
class StrategyBufferOpNode(ASTNode):
    def cache(self) -> numpy.ndarray[numpy.float64[m, n]]:
        ...
    def lag(self, arg0: int) -> StrategyBufferOpNode:
        ...
class StrategyGrid:
    def cols(self) -> int:
        ...
    def enableTracerHistory(self, arg0: TracerType) -> bool:
        ...
    def getTracer(self, arg0: int, arg1: int) -> Tracer | None:
        ...
    def meanReturn(self) -> float:
        ...
    def rows(self) -> int:
        ...
class StrategyMonthlyRunnerNode(TriggerNode):
    @staticmethod
    def make(exchange: atlas_internal.core.Exchange, eom_trigger: bool = False) -> TriggerNode:
        ...
class StrategyNode:
    @staticmethod
    def make(allocation: AllocationBaseNode, portfolio: atlas_internal.core.Portfolio) -> StrategyNode:
        ...
    def setTrigger(self, arg0: TriggerNode) -> None:
        ...
    def setWarmupOverride(self, arg0: int) -> None:
        ...
class SumObserverNode(AssetObserverNode):
    def __init__(self, arg0: str, arg1: StrategyBufferOpNode, arg2: int) -> None:
        ...
class Tracer:
    def getHistory(self, arg0: TracerType) -> numpy.ndarray[numpy.float64[m, 1]]:
        ...
    def getOrders(self) -> list[atlas_internal.core.Order]:
        ...
class TracerType:
    """
    Members:
    
      NLV
    
      WEIGHTS
    
      VOLATILITY
    
      ORDERS_EAGER
    """
    NLV: typing.ClassVar[TracerType]  # value = <TracerType.NLV: 0>
    ORDERS_EAGER: typing.ClassVar[TracerType]  # value = <TracerType.ORDERS_EAGER: 3>
    VOLATILITY: typing.ClassVar[TracerType]  # value = <TracerType.VOLATILITY: 1>
    WEIGHTS: typing.ClassVar[TracerType]  # value = <TracerType.WEIGHTS: 2>
    __members__: typing.ClassVar[dict[str, TracerType]]  # value = {'NLV': <TracerType.NLV: 0>, 'WEIGHTS': <TracerType.WEIGHTS: 2>, 'VOLATILITY': <TracerType.VOLATILITY: 1>, 'ORDERS_EAGER': <TracerType.ORDERS_EAGER: 3>}
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
class TradeLimitNode(ASTNode):
    @staticmethod
    def getStopLoss(arg0: TradeLimitNode) -> float:
        ...
    @staticmethod
    def getTakeProfit(arg0: TradeLimitNode) -> float:
        ...
    @staticmethod
    def setStopLoss(arg0: TradeLimitNode, arg1: float) -> None:
        ...
    @staticmethod
    def setTakeProfit(arg0: TradeLimitNode, arg1: float) -> None:
        ...
    def stopLossGetter(self) -> int:
        ...
    def stopLossSetter(self) -> int:
        ...
    def takeProfitGetter(self) -> int:
        ...
    def takeProfitSetter(self) -> int:
        ...
class TradeLimitType:
    """
    Members:
    
      STOP_LOSS
    
      TAKE_PROFIT
    """
    STOP_LOSS: typing.ClassVar[TradeLimitType]  # value = <TradeLimitType.STOP_LOSS: 1>
    TAKE_PROFIT: typing.ClassVar[TradeLimitType]  # value = <TradeLimitType.TAKE_PROFIT: 2>
    __members__: typing.ClassVar[dict[str, TradeLimitType]]  # value = {'STOP_LOSS': <TradeLimitType.STOP_LOSS: 1>, 'TAKE_PROFIT': <TradeLimitType.TAKE_PROFIT: 2>}
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
class TriggerNode:
    def getMask(self) -> numpy.ndarray[numpy.int32[m, 1]]:
        ...
class TsArgMaxObserverNode(AssetObserverNode):
    def __init__(self, arg0: str, arg1: StrategyBufferOpNode, arg2: int) -> None:
        ...
class VarianceObserverNode(AssetObserverNode):
    def __init__(self, arg0: str, arg1: StrategyBufferOpNode, arg2: int) -> None:
        ...
ABS: AssetFunctionType  # value = <AssetFunctionType.ABS: 3>
ADD: AssetOpType  # value = <AssetOpType.ADD: 0>
AND: LogicalType  # value = <LogicalType.AND: 0>
CONDITIONAL_SPLIT: AllocationType  # value = <AllocationType.CONDITIONAL_SPLIT: 1>
DIVIDE: AssetOpType  # value = <AssetOpType.DIVIDE: 3>
EQUAL: AssetCompType  # value = <AssetCompType.EQUAL: 0>
FULL: CovarianceType  # value = <CovarianceType.FULL: 0>
GREATER: AssetCompType  # value = <AssetCompType.GREATER: 2>
GREATER_EQUAL: AssetCompType  # value = <AssetCompType.GREATER_EQUAL: 4>
GREATER_THAN: ExchangeViewFilterType  # value = <ExchangeViewFilterType.GREATER_THAN: 0>
INCREMENTAL: CovarianceType  # value = <CovarianceType.INCREMENTAL: 1>
LESS: AssetCompType  # value = <AssetCompType.LESS: 3>
LESS_EQUAL: AssetCompType  # value = <AssetCompType.LESS_EQUAL: 5>
LESS_THAN: ExchangeViewFilterType  # value = <ExchangeViewFilterType.LESS_THAN: 1>
LOG: AssetFunctionType  # value = <AssetFunctionType.LOG: 4>
LOWER_TRIANGULAR: GridType  # value = <GridType.LOWER_TRIANGULAR: 2>
MULTIPLY: AssetOpType  # value = <AssetOpType.MULTIPLY: 2>
NEXTREME: EVRankType  # value = <EVRankType.NEXTREME: 2>
NLARGEST: EVRankType  # value = <EVRankType.NLARGEST: 0>
NLV: TracerType  # value = <TracerType.NLV: 0>
NOT_EQUAL: AssetCompType  # value = <AssetCompType.NOT_EQUAL: 1>
NSMALLEST: EVRankType  # value = <EVRankType.NSMALLEST: 1>
OR: LogicalType  # value = <LogicalType.OR: 1>
ORDERS_EAGER: TracerType  # value = <TracerType.ORDERS_EAGER: 3>
POWER: AssetFunctionType  # value = <AssetFunctionType.POWER: 1>
SIGN: AssetFunctionType  # value = <AssetFunctionType.SIGN: 0>
STOP_LOSS: TradeLimitType  # value = <TradeLimitType.STOP_LOSS: 1>
SUBTRACT: AssetOpType  # value = <AssetOpType.SUBTRACT: 1>
TAKE_PROFIT: TradeLimitType  # value = <TradeLimitType.TAKE_PROFIT: 2>
UNIFORM: AllocationType  # value = <AllocationType.UNIFORM: 0>
UPPER_TRIANGULAR: GridType  # value = <GridType.UPPER_TRIANGULAR: 1>
VOLATILITY: TracerType  # value = <TracerType.VOLATILITY: 1>
WEIGHTS: TracerType  # value = <TracerType.WEIGHTS: 2>
