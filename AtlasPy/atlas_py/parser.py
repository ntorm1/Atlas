import logging
import os
import sys
import inspect
import importlib
from typing import *
import tomllib
from dataclasses import dataclass

from atlas_internal.core import Hydra, Exchange, Strategy, MetaStrategy, Allocator
from .strategy import PyStrategy, PyMetaStrategy
from .atlas_logging import CustomLogger


@dataclass
class ExchangeConfig:
    id: str
    path: str
    datetime_format: str


@dataclass
class StrategyConfig:
    id: str
    parent_id: str
    portfolio_weight: float
    exchange_id: str


class Parser:
    _hydra: Hydra = None
    _hydra_config_dir: str = None
    _toml: Dict[str, Any] = None
    _logger = CustomLogger(__name__)

    def __init__(self, hydra_config_dir: Hydra, logging_level: int = logging.ERROR):
        self._hydra_config_dir = hydra_config_dir
        self._hydra = Hydra()
        self._parse()
        self._logger = CustomLogger(__name__, logging_level=logging_level)

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
            self._logger.error(f"An error occurred: {type(e).__name__}: {e}")
            raise e

        self._load_exchanges()
        self._hydra.build()
        self._load_strategies()

    def _load_strategies(self) -> None:
        sub_dirs = [
            os.path.join(self._hydra_config_dir, d)
            for d in os.listdir(self._hydra_config_dir)
            if os.path.isdir(os.path.join(self._hydra_config_dir, d))
        ]

        for sub_dir in sub_dirs:
            self._load_strategy(strategy_dir=sub_dir, parent=None)

    def _load_strategy(self, strategy_dir: str, parent: Optional[Strategy]) -> None:
        assert os.path.isdir(strategy_dir)
        strategy_config_path = os.path.join(strategy_dir, "Strategy.toml")
        py_files = [f for f in os.listdir(strategy_dir) if f.endswith(".py")]
        if len(py_files) == 0:
            self._logger.info(f"no py files in {strategy_dir}")
            return None

        if not os.path.isfile(strategy_config_path):
            self._logger.error(f"missing Strategy.toml: {strategy_config_path}")
            raise FileNotFoundError(f"missing Strategy.toml: {strategy_config_path}")
        try:
            with open(strategy_config_path, "rb") as f:
                strategy_toml = tomllib.load(f)
            strategy_config = StrategyConfig(**strategy_toml)
        except Exception as e:
            self._logger.error(
                f"Failed to load {strategy_config_path} config: {type(e).__name__}: {e}"
            )
            raise e
        if len(py_files) != 1:
            self._logger.error(
                f"expected 1 .py file in {strategy_dir}, found: {py_files}"
            )
            raise FileNotFoundError(f"expected 1 .py file in {strategy_dir}")
        strategy = self._load_strategy_module(
            strategy_config=strategy_config,
            strategy_dir=strategy_dir,
            py_file=py_files[0],
            parent=parent,
        )
        try:
            if parent is None:
                self._logger.info(
                    f"Adding strategy {strategy_config.id}, type: {type(strategy)}, parent: {type(parent)}"
                )
                strategy = self._hydra.addStrategy(strategy, replace_if_exists=True)
            else:
                self._logger.info(
                    f"Adding strategy {strategy_config.id}, type: {type(strategy)}, parent: {type(parent)}"
                )
                if not isinstance(parent, MetaStrategy):
                    self._logger.error(
                        f"expected parent to be MetaStrategy, got: {type(parent)}"
                    )
                    raise ValueError(f"expected parent to be PyMetaStrategy")
                if not isinstance(strategy, Allocator):
                    self._logger.error(
                        f"expected strategy to be Allocator, got: {type(strategy)}"
                    )
                    raise ValueError(f"expected strategy to be PyStrategy")
                strategy = parent.addStrategy(strategy, False)
            self._logger.info(f"Added strategy {strategy_config.id}")
        except Exception as e:
            self._logger.error(
                f"Failed to add strategy {strategy_config.id}: {type(e).__name__}: {e}"
            )
            raise e
        sub_dirs = [
            os.path.join(strategy_dir, d)
            for d in os.listdir(strategy_dir)
            if os.path.isdir(os.path.join(strategy_dir, d))
        ]
        for sub_dir in sub_dirs:
            self._load_strategy(strategy_dir=sub_dir, parent=strategy)

    def _load_strategy_module(
        self,
        strategy_config: StrategyConfig,
        strategy_dir: str,
        py_file: str,
        parent: Optional[Strategy],
    ) -> Strategy:
        sys.path.insert(0, strategy_dir)
        module_name = os.path.basename(py_file).replace(".py", "")
        module = importlib.import_module(module_name)
        classes = inspect.getmembers(module, inspect.isclass)
        strategy_class = None
        for _, cls in classes:
            if issubclass(cls, PyStrategy) or issubclass(cls, PyMetaStrategy):
                strategy_class = cls
        py_file_path = os.path.join(strategy_dir, py_file)
        module_name = os.path.basename(py_file).replace(".py", "")
        module = importlib.import_module(module_name)
        classes = inspect.getmembers(module, inspect.isclass)
        strategy_class = None
        for name, cls in classes:
            if issubclass(cls, PyStrategy) or issubclass(cls, PyMetaStrategy):
                strategy_class = cls
        if strategy_class is None:
            self._logger.error(f"missing PyStrategy in {py_file_path}")
            raise FileNotFoundError(f"missing PyStrategy in {py_file_path}")
        if issubclass(cls, PyStrategy) and parent is None:
            self._logger.error(f"expected PyMetaStrategy in {py_file_path}")
            raise FileNotFoundError(f"expected PyMetaStrategy in {py_file_path}")
        exchange = self._hydra.getExchange(strategy_config.exchange_id)
        return strategy_class(
            exchange=exchange,
            parent_strategy=parent,
            name=strategy_config.id,
            portfolio_weight=strategy_config.portfolio_weight,
        )

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
