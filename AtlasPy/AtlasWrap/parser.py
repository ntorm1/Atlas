import logging
import os
import sys
import importlib
from typing import *
import tomllib

from AtlasPy.core import Hydra, Exchange, Portfolio
import AtlasPy

class Parser:
    _hydra: Hydra = None
    _hydra_config_dir: str = None
    _toml: Dict[str, Any] = None

    def __init__(
            self,
            hydra_config_dir: Hydra
        ) -> None:
        self._hydra_config_dir = hydra_config_dir
        self._hydra = Hydra()
        self._parse()

    def getHydra(
            self
            ) -> Hydra:
        return self._hydra

    def _parse(
            self,
            ) -> None:
        if not os.path.isdir(self._hydra_config_dir):
            raise FileNotFoundError(f"expected dir: {self._hydra_config_dir}")           

        _hydra_config_path = os.path.join(self._hydra_config_dir, "Hydra.toml")

        if not os.path.isfile(_hydra_config_path):
            raise FileNotFoundError(f"missing Hydra.toml: {_hydra_config_path}")
        
        self._load_toml(_hydra_config_path)
        self._validate_toml()

    def _load_toml(
            self,
            file_path: str
            ) -> None:
        try:
            with open(file_path, 'rb') as f:
                self._toml = tomllib.load(f)
        except Exception as e:
            logging.error(f"An error occurred: {type(e).__name__}: {e}")
            raise e
        
        self._load_exchanges()
        self._load_portfolios()
        self._hydra.build()
        self._load_strategies()

    def _load_strategies(
            self
        ) -> None:
        sys.path.insert(0, self._hydra_config_dir)
        strategy_dirs = [d for d in os.listdir(self._hydra_config_dir) if os.path.isdir(os.path.join(self._hydra_config_dir, d))]
        if len(strategy_dirs) == 0:
            raise FileNotFoundError(f"no strategy dirs found in: {self._hydra_config_dir}")
        for strategy_dir in strategy_dirs:
            logging.info(f"loading strategy: {strategy_dir}")
            strategy_path = os.path.join(self._hydra_config_dir, strategy_dir, "strategy.py")
            strategy_toml = os.path.join(self._hydra_config_dir, strategy_dir, "strategy.toml")
            with open(strategy_toml, 'rb') as f:
                strategy_toml = tomllib.load(f)

            strategy_id = strategy_toml.get("strategy")["id"]
            portfolio_id = strategy_toml.get("portfolio")["id"]
            exchange_id = strategy_toml.get("exchange")["id"]
            if not strategy_id or not portfolio_id or not exchange_id:
                raise ValueError(f"strategy: {strategy_dir} missing id, portfolio_id, or exchange_id")
            
            exchange = self._hydra.getExchange(exchange_id)
            portfolio = self._hydra.getPortfolio(portfolio_id)

            # check that strategy_path exists
            if not os.path.isfile(strategy_path):
                raise FileNotFoundError(f"missing strategy.py: {strategy_path}")
            module_name = f"{strategy_dir}.strategy"
            strategy_module = importlib.import_module(module_name)

            # check that strategy_path defines a function called ast
            if not hasattr(strategy_module, 'ast') or not callable(getattr(strategy_module, 'ast')):
                raise TypeError(f"missing function 'ast' in: {strategy_path}")

            # check that ast function has type hints and accepts exchange and portfolio
            ast_function = getattr(strategy_module, 'ast')
            if hasattr(ast_function, '__annotations__'):
                annotations = ast_function.__annotations__
                print(annotations)
                if 'exchange' not in annotations or annotations['exchange'] != Exchange:
                    logging.error(f"annotations: {annotations}")
                    raise TypeError("Function 'ast' must accept 'exchange' keyword argument of type Exchange")
            else:
                logging.error(f"annotations: {annotations}")
                raise TypeError("Function 'ast' must have type hints")
            
            # check function returns an AllocationNode
            if not hasattr(ast_function, '__annotations__') or 'return' not in ast_function.__annotations__ or ast_function.__annotations__['return'] != AtlasPy.ast.AllocationNode:
                logging.error(f"annotations: {annotations}")
                raise TypeError("Function 'ast' must return an AtlasPy.ast.AllocationNode")

            # build the allocation node
            allocation = ast_function(
                exchange=exchange
            )  

            # register the strategy
            strategy_node_signal = AtlasPy.ast.StrategyNode.make(allocation, portfolio)
            _ = self._hydra.addStrategy(AtlasPy.core.Strategy(strategy_id, strategy_node_signal, 1.0), True)
            logging.info(f"loaded strategy: {strategy_dir}")

    def _load_portfolios(
            self
            ) -> None:
        if not "portfolios" in self._toml or not isinstance(self._toml["portfolios"], list):
            raise ValueError("""missing portfolios in Hydra.toml, expected: 
                    [portfolios]
                    [[portfolios.ids]]
                    id = "test"
                    exchange_id = "test"
                    starting_cash = 100.0
                """
            )
        
        for portfolio in self._toml["portfolios"]:
            if not isinstance(portfolio, dict):
                raise ValueError("expected portfolio to be a dict")
            id = portfolio.get("id")
            exchange_id = portfolio.get("exchange_id")
            starting_cash = portfolio.get("starting_cash")
            if not id or not exchange_id or not starting_cash:
                raise ValueError("portfolio missing id, exchange_id, or starting_cash")
            exchange = self._hydra.getExchange(exchange_id)
            self._hydra.addPortfolio(id, exchange, starting_cash)

    def _load_exchanges(
            self
            ) -> None:
        if not "exchanges" in self._toml or not isinstance(self._toml["exchanges"], list):
            raise ValueError("""missing exchanges in Hydra.toml, expected: 
                    [exchanges]
                    [[exchanges.ids]]
                    id = "test"
                    path = "C:/Users/.../exchange1"
                    datetime_format = "%Y-%m-%d %H:%M:%S"
                """
            )
        
        for exchange in self._toml["exchanges"]:
            if not isinstance(exchange, dict):
                raise ValueError("expected exchange to be a dict")
            id = exchange.get("id")
            path = exchange.get("path")
            datetime_format = exchange.get("datetime_format")
            if not id or not path or not datetime_format:
                raise ValueError("exchange missing id, path, or datetime_format")
            self._hydra.addExchange(id, path, datetime_format)
        

    def _validate_toml(
            self
            ) -> None:
        pass