import logging
import os
import sys
from typing import *
import tomllib

from AtlasPy.core import Hydra

class Parser:
    _hydra: Hydra
    _toml: Dict[str, Any]

    def __init__(
            self,
            hydra: Hydra
        ) -> None:
        self._hydra = hydra

    def parse(
            self,
            file_path: str,
            ) -> Dict[str, Any]:
        self._load_toml(file_path)
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
        
    def _validate_toml(
            self,
            ) -> None:
        self._validate_strategy()
        self._validate_portfolio()

    def _validate_strategy(self) -> None:
        strategy = self._toml.get('strategy')

        if not strategy or not isinstance(strategy, dict):
            logging.error("Invalid strategy format in the TOML file.")
            raise ValueError("Invalid strategy format in the TOML file.")

        strategy_id = strategy.get('id')

        if not strategy_id or not isinstance(strategy_id, str):
            logging.error("Invalid strategy ID format.")
            raise ValueError("Invalid strategy ID format.")
        return None
    
    def _validate_portfolio(
            self
            ) -> None:
        portfolio = self._toml.get('portfolio')

        if not portfolio or not isinstance(portfolio, dict):
            logging.error("Invalid portfolio format in the TOML file.")
            raise ValueError("Invalid portfolio format in the TOML file.")

        portfolio_id = portfolio.get('id')

        if not portfolio_id or not isinstance(portfolio_id, str):
            logging.error("Invalid portfolio ID format.")
            raise ValueError("Invalid portfolio ID format.")
    
        try:
            portfolio = self._hydra.getPortfolio(portfolio_id)
        except Exception as e:
            logging.error(f"An error occurred: {type(e).__name__}: {e}")
            raise e
        return None