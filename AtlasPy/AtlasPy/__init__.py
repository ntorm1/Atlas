import logging
import sys

logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)

# python -m pybind11_stubgen AtlasPy --ignore-all-errors -o C:\Users\natha\OneDrive\Desktop\C++\Atlas\AtlasPy\AtlasWrap
ATLAS_CORE_PATH = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Release"
sys.path.append(ATLAS_CORE_PATH)

from atlas_internal.core import Hydra, Strategy, MetaStrategy
from atlas_internal.ast import *
from atlas_internal.model import *
from .parser import Parser
