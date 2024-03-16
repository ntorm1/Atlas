import os
import sys
import logging
from abc import ABC, abstractmethod


import atlas_internal
from atlas_internal.core import Hydra, Exchange, Portfolio, Strategy


class PyStrategy(Strategy):
    portfolio: Portfolio = None
    exchange: Exchange = None

    def __init__(
        self,
        exchange: Exchange,
        portfolio,
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(name, portfolio_weight)
        self.exchange = exchange
        self.portfolio = portfolio

    @abstractmethod
    def loadAST(self) -> atlas_internal.ast.StrategyBufferOpNode:
        pass


class ImmediateStrategy(PyStrategy):
    ast: atlas_internal.ast.StrategyBufferOpNode = None

    def __init__(
        self,
        exchange: Exchange,
        portfolio,
        name: str,
        portfolio_weight: float,
        ast: atlas_internal.ast.StrategyBufferOpNode,
    ) -> None:
        super().__init__(
            exchange=exchange,
            portfolio=portfolio,
            name=name,
            portfolio_weight=portfolio_weight,
        )
        self.exchange = exchange
        self.portfolio = portfolio
        self.ast = ast

    def loadAST(self) -> atlas_internal.ast.StrategyBufferOpNode:
        return self.ast
