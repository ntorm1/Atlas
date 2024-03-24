import logging
import os
import inspect
import importlib
from typing import *
import tomllib
from dataclasses import dataclass

from atlas_internal.core import Hydra, Exchange
from .strategy import PyStrategy


@dataclass
class ExchangeConfig:
    id: str
    path: str
    datetime_format: str


@dataclass
class StrategyConfig:
    id: str
    parent_id: str
    weight: float
    exchange_id: str


class Parser:
    _hydra: Hydra = None
    _hydra_config_dir: str = None
    _toml: Dict[str, Any] = None

    def __init__(self, hydra_config_dir: Hydra) -> None:
        self._hydra_config_dir = hydra_config_dir
        self._hydra = Hydra()
        self._parse()

    def getHydra(self) -> Hydra:
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

    def _load_toml(self, file_path: str) -> None:
        try:
            with open(file_path, "rb") as f:
                self._toml = tomllib.load(f)
        except Exception as e:
            logging.error(f"An error occurred: {type(e).__name__}: {e}")
            raise e

        self._load_exchanges()
        self._hydra.build()
        self._load_strategies()

    def _load_strategies(self) -> None:
        sub_dirs = [f.path for f in os.scandir(self._hydra_config_dir) if f.is_dir()]
        for dir in sub_dirs:
            # get .toml and .py file from dir, make sure exactly one of each
            strategy_toml = None
            strategy_py = None
            for f in os.scandir(dir):
                if f.name.endswith(".toml"):
                    if strategy_toml is not None:
                        raise ValueError(f"expected one .toml file in {dir}")
                    strategy_toml = f.path
                elif f.name.endswith(".py"):
                    if strategy_py is not None:
                        raise ValueError(f"expected one .py file in {dir}")
                    strategy_py = f.path
            if strategy_toml is None or strategy_py is None:
                raise ValueError(f"expected one .toml and one .py file in {dir}")
            self._load_strategy(strategy_toml, strategy_py)

    def _load_strategy(self, strategy_toml: str, file_path: str) -> None:
        # find all classes in file_path that inherit from PyStrategy
        with open(file_path, "rb") as f:
            strategy_toml = tomllib.load(f)
        strategy_config = StrategyConfig(**strategy_toml)
        exchange = self._hydra.getExchange(strategy_config.exchange_id)
        strategy_module = importlib.import_module(file_path)
        for _, obj in inspect.getmembers(strategy_module):
            if inspect.isclass(obj) and issubclass(obj, PyStrategy):
                strategy = obj(exchange, strategy_config.weight)
                self._hydra.addStrategy(strategy_config.id, strategy)

    def _load_exchanges(self) -> None:
        if not "exchanges" in self._toml or not isinstance(
            self._toml["exchanges"], list
        ):
            raise ValueError(
                """missing exchanges in Hydra.toml, expected: 
                    [exchanges]
                    [[exchanges.ids]]
                    id = "test"
                    path = "C:/Users/.../exchange1"
                    datetime_format = "%Y-%m-%d %H:%M:%S"
                """
            )
        exchanges = [ExchangeConfig(**exchange) for exchange in self._toml["exchanges"]]
        for exchange in exchanges:
            self._hydra.addExchange(
                exchange.id, exchange.path, exchange.datetime_format
            )

    def _validate_toml(self) -> None:
        pass
