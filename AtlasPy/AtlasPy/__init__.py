import logging
import sys

# python -m pybind11_stubgen atlas_internal --ignore-all-errors -o C:\Users\natha\OneDrive\Desktop\C++\Atlas\AtlasPy\AtlasWrap
ATLAS_CORE_PATH = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Debug"
sys.path.append(ATLAS_CORE_PATH)

from atlas_internal.core import Hydra, Strategy, MetaStrategy
from atlas_internal.ast import *
from atlas_internal.model import *
from .parser import Parser
