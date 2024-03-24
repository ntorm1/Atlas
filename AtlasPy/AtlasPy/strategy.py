from typing import *
from abc import ABC, abstractmethod

import atlas_internal
from atlas_internal.core import Hydra, Exchange, Allocator, Strategy, MetaStrategy


class PyStrategy(Strategy):
    parent_strategy: Allocator = None
    exchange: Exchange = None

    def __init__(
        self,
        exchange: Exchange,
        parent_strategy: Allocator,
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(name, exchange, parent_strategy, portfolio_weight)
        self.exchange = exchange
        self.parent_strategy = parent_strategy

    @abstractmethod
    def loadAST(self) -> atlas_internal.ast.StrategyNode:
        pass


class PyMetaStrategy(MetaStrategy):
    parent_strategy: Allocator = None
    exchange: Exchange = None

    def __init__(
        self,
        exchange: Exchange,
        parent_strategy: Optional[Allocator],
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(name, exchange, parent_strategy, portfolio_weight)

    @abstractmethod
    def allocate(slef) -> None:
        pass


class ImmediateStrategy(PyStrategy):
    ast: atlas_internal.ast.StrategyNode = None

    def __init__(
        self,
        exchange: Exchange,
        parent_strategy: Allocator,
        name: str,
        portfolio_weight: float,
        ast: atlas_internal.ast.StrategyNode,
    ) -> None:
        super().__init__(
            exchange=exchange,
            parent_strategy=parent_strategy,
            name=name,
            portfolio_weight=portfolio_weight,
        )
        self.exchange = exchange
        self.ast = ast

    def loadAST(self) -> atlas_internal.ast.StrategyNode:
        return self.ast
