import logging
import os
import sys
import importlib
from typing import *
import tomllib
from dataclasses import dataclass

from atlas_internal.core import Hydra, Exchange


@dataclass
class ExchangeConfig:
    id: str
    path: str
    datetime_format: str


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
        sys.path.insert(0, self._hydra_config_dir)
        # get all py files in the directory
        strategy_files = [
            f for f in os.listdir(self._hydra_config_dir) if f.endswith(".py")
        ]

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
