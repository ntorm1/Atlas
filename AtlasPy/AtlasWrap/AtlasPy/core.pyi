import internal.ast
from __future__ import annotations
import numpy
import typing

__all__ = [
    "CommisionManager",
    "Exchange",
    "Hydra",
    "Order",
    "Portfolio",
    "Strategy",
    "Trade",
]

class CommisionManager:
    def setCommissionPct(self, arg0: float) -> None: ...
    def setFixedCommission(self, arg0: float) -> None: ...

class Exchange:
    def enableNodeCache(
        self, arg0: str, arg1: internal.ast.StrategyBufferOpNode, arg2: bool
    ) -> None: ...
    def getAssetIndex(self, arg0: str) -> int | None: ...
    def getAssetMap(self) -> dict[str, int]: ...
    def getCovarianceNode(self, arg0: str, arg1: ..., arg2: int, arg3: ...) -> ...: ...
    def getCurrentTimestamp(self) -> int: ...
    def getMarketReturns(
        self, row_offset: int = 0
    ) -> numpy.ndarray[numpy.float64[m, 1], numpy.ndarray.flags.writeable]: ...
    def getName(self) -> str:
        """
        get unique id of the exchange
        """

    def getObserver(self, arg0: str) -> any | None: ...
    def getTimestamps(self) -> list[int]: ...
    def registerModel(self, arg0: ...) -> None: ...
    def registerObserver(self, arg0: ...) -> ...: ...

class Hydra:
    def __init__(self) -> None: ...
    def addExchange(
        self, name: str, source: str, datetime_format: str | None = None
    ) -> ...: ...
    def addPortfolio(self, arg0: str, arg1: ..., arg2: float) -> Portfolio: ...
    def addStrategy(self, strategy: ..., replace_if_exists: bool = False) -> ...: ...
    def build(self) -> None: ...
    def getExchange(self, arg0: str) -> ...: ...
    def getPortfolio(self, arg0: str) -> Portfolio: ...
    def removeStrategy(self, arg0: str) -> None: ...
    def reset(self) -> None: ...
    def run(self) -> None: ...
    def step(self) -> None: ...

class Order:
    def __init__(
        self, arg0: int, arg1: int, arg2: int, arg3: float, arg4: float
    ) -> None: ...
    def to_dict(self) -> dict: ...
    @property
    def asset_id(self) -> int: ...
    @property
    def fill_price(self) -> float: ...
    @property
    def fill_time(self) -> int: ...
    @property
    def quantity(self) -> float: ...
    @property
    def strategy_id(self) -> int: ...

class Portfolio:
    def getName(self) -> str:
        """
        get unique id of the portfolio
        """

class Strategy:
    def __init__(
        self, arg0: str, arg1: internal.ast.StrategyNode, arg2: float
    ) -> None: ...
    def enableTracerHistory(self, arg0: internal.ast.TracerType) -> None: ...
    def getAllocationBuffer(self) -> numpy.ndarray[numpy.float64[m, 1]]: ...
    def getHistory(
        self, arg0: internal.ast.TracerType
    ) -> numpy.ndarray[numpy.float64[m, 1]]: ...
    def getNLV(self) -> float: ...
    def getName(self) -> str: ...
    def getTracer(self) -> internal.ast.Tracer: ...
    def getWeightHistory(self) -> numpy.ndarray[numpy.float64[m, n]]: ...
    def initCommissionManager(self) -> CommisionManager: ...
    def setGridDimmensions(
        self,
        dimensions: tuple[any, ...],
        grid_type: internal.ast.GridType | None = None,
    ) -> internal.ast.StrategyGrid: ...
    def setVolTracer(self, arg0: internal.ast.CovarianceNodeBase) -> None: ...

class Trade:
    def __init__(
        self,
        arg0: int,
        arg1: int,
        arg2: int,
        arg3: int,
        arg4: float,
        arg5: float,
        arg6: float,
    ) -> None: ...
    def to_dict(self) -> dict: ...
    @property
    def asset_id(self) -> int: ...
    @property
    def close_price(self) -> float: ...
    @property
    def close_time(self) -> int: ...
    @property
    def open_price(self) -> float: ...
    @property
    def open_time(self) -> int: ...
    @property
    def quantity(self) -> float: ...
    @property
    def strategy_id(self) -> int: ...
