import ast
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
        self, arg0: str, arg1: ast.StrategyBufferOpNode, arg2: bool
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

    def getObserver(self, arg0: str) -> ... | None: ...
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
    """

    Abstract base class for strategies to override

    Example:
        >>> strategy = Strategy("name", 0.5)
        >>> strategy.loadAST(ast)
        >>> nlv = strategy.getNLV()

    Attributes:
        - name (str): The name of the strategy.
        - value (float): The value of the strategy.

    """

    def __init__(self, arg0: str, arg1: float) -> None:
        """
        Constructor for Strategy class.

        Args:
            name (str): The name of the strategy.
            value (float): The portfolio weight of the strategy.
        """

    def enableTracerHistory(self, arg0: ast.TracerType) -> None:
        """
        Enable tracer history for the strategy.
        """

    def getAllocationBuffer(self) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Get the allocation buffer for the strategy.

        Returns:
            AllocationBuffer: The allocation buffer object.
        """

    def getHistory(self, arg0: ast.TracerType) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Get the history for the strategy.

        Returns:
            History: The history object.
        """

    def getNLV(self) -> float:
        """
        Get the NLV (Net Liquidation Value) of the strategy.

        Returns:
            float: The Net Liquidation Value of the strategy.
        """

    def getName(self) -> str:
        """
        Get the name of the strategy.

        Returns:
            str: The name of the strategy.
        """

    def getTracer(self) -> ast.Tracer:
        """
        Get the tracer for the strategy containing information about the strategy state.

        Returns:
            Tracer: The tracer object.
        """

    def getWeightHistory(self) -> numpy.ndarray[numpy.float64[m, n]]:
        """
        Get the weight history for the strategy.

        Returns:
            WeightHistory: The weight history object.
        """

    def initCommissionManager(self) -> CommisionManager:
        """
        Initialize the commission manager for the strategy.
        """

    def loadAST(self) -> ast.StrategyNode:
        """
        Pure virtual ast loader to be defined by derived classes

        Args:
            ast (AST): The Abstract Syntax Tree representing the strategy.
        """

    def setGridDimmensions(
        self, dimensions: tuple[..., ...], grid_type: ast.GridType | None = None
    ) -> ast.StrategyGrid:
        """
        Set the grid dimensions for the strategy.

        Args:
            dimensions (tuple): The dimensions of the grid.
            grid_type (Optional[str]): The type of the grid. Defaults to None.
        """

    def setVolTracer(self, arg0: ast.CovarianceNodeBase) -> None:
        """
        Set the volatility tracer for the strategy.
        """

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
