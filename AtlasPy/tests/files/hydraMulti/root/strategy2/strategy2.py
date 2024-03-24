from atlas_py import atlas_internal
from atlas_py.strategy import *


class TestStrategy2(PyStrategy):
    _ast: atlas_internal.ast.StrategyNode = None

    def __init__(
        self,
        exchange: Exchange,
        parent_strategy: Allocator,
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(
            exchange=exchange,
            parent_strategy=parent_strategy,
            name=name,
            portfolio_weight=portfolio_weight,
        )
        close = atlas_internal.ast.AssetReadNode.make("close", 0, self.exchange)
        ev = atlas_internal.ast.ExchangeViewNode.make(self.exchange, close)
        allocation = atlas_internal.ast.AllocationNode.make(ev)
        self._ast = atlas_internal.ast.StrategyNode.make(allocation)

    def loadAST(self) -> atlas_internal.ast.StrategyNode:
        return self._ast
