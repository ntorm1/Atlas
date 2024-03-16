import os
import sys
import logging

sys.path.insert(
    0,
    os.path.abspath(
        os.path.join(os.path.dirname(os.path.dirname(__file__)), "../../..")
    ),
)

from AtlasWrap import AtlasPy
from AtlasWrap.strategy import *


class ObserverTestStrategy(PyStrategy):
    def __init__(
        self,
        exchange: Exchange,
        portfolio,
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(exchange, portfolio, name, portfolio_weight)

    def loadAST(self) -> AtlasPy.ast.AllocationNode:
        window = 5
        close = AtlasPy.ast.AssetReadNode.make("Close", 0, self.exchange)
        prev_close = AtlasPy.ast.AssetReadNode.make("Close", -1, self.exchange)
        change = AtlasPy.ast.AssetOpNode.make(
            close, prev_close, AtlasPy.ast.AssetOpType.SUBTRACT
        )
        close_arg_max = self.exchange.registerObserver(
            AtlasPy.ast.TsArgMaxObserverNode("arg_max", change, window)
        )
        close_max = self.exchange.registerObserver(
            AtlasPy.ast.MaxObserverNode("max", change, window)
        )
        self.exchange.enableNodeCache("close_arg_max", close_arg_max, False)
        self.exchange.enableNodeCache("close_max", close_max, False)

        ev = AtlasPy.ast.exchangeViewNode.make(self.exchange, close)
        allocation = AtlasPy.ast.AllocationNode.make(ev)
        return AtlasPy.ast.StrategyNode.make(allocation, self.portfolio)
