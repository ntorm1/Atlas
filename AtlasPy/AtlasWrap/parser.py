import logging
import os
import sys
from typing import *
import tomllib

from AtlasPy.core import Hydra

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