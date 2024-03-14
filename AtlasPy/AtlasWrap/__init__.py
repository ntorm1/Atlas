import logging
import sys

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')

ATLAS_CORE_PATH = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Debug"
sys.path.append(ATLAS_CORE_PATH)

import AtlasPy

from .parser import Parser

