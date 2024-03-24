import os
import sys
import logging

sys.path.insert(
    0,
    os.path.abspath(
        os.path.join(os.path.dirname(os.path.dirname(__file__)), "../../..")
    ),
)

from atlas_py import atlas_internal
from atlas_py.strategy import *


class ObserverTestStrategy(PyStrategy):
    def __init__(
        self,
        exchange: Exchange,
        portfolio,
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(exchange, portfolio, name, portfolio_weight)

    def loadAST(self) -> atlas_internal.ast.AllocationNode:
        window = 5
        close = atlas_internal.ast.AssetReadNode.make("Close", 0, self.exchange)
        prev_close = atlas_internal.ast.AssetReadNode.make("Close", -1, self.exchange)
        change = atlas_internal.ast.AssetOpNode.make(
            close, prev_close, atlas_internal.ast.AssetOpType.SUBTRACT
        )
        close_arg_max = self.exchange.registerObserver(
            atlas_internal.ast.TsArgMaxObserverNode("arg_max", change, window)
        )
        close_max = self.exchange.registerObserver(
            atlas_internal.ast.MaxObserverNode("max", change, window)
        )
        self.exchange.enableNodeCache("close_arg_max", close_arg_max, False)
        self.exchange.enableNodeCache("close_max", close_max, False)

        ev = atlas_internal.ast.exchangeViewNode.make(self.exchange, close)
        allocation = atlas_internal.ast.AllocationNode.make(ev)
        return atlas_internal.ast.StrategyNode.make(allocation, self.portfolio)
