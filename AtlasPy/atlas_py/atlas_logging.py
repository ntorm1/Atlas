import logging
import sys


class CustomFormatter(logging.Formatter):
    """Custom formatter to add colors to log messages"""

    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE = range(8)

    RESET_SEQ = "\033[0m"
    COLOR_SEQ = "\033[1;%dm"
    BOLD_SEQ = "\033[1m"
    UNDERLINE_SEQ = "\033[4m"

    COLORS = {
        "DEBUG": BLUE,
        "INFO": GREEN,
        "WARNING": YELLOW,
        "ERROR": RED,
        "CRITICAL": MAGENTA,
    }

    def format(self, record):
        levelname = record.levelname
        if levelname in self.COLORS:
            color_code = self.COLORS[levelname]
            record.levelname = f"{self.COLOR_SEQ % (30 + color_code)}{record.levelname}{self.RESET_SEQ}"
        return super().format(record)


class CustomLogger(logging.Logger):
    def __init__(self, name, log_file=None, logging_level: int = logging.DEBUG):
        super().__init__(name)
        self.setLevel(logging_level)

        formatter = CustomFormatter(
            fmt="%(asctime)s - %(levelname)s - %(filename)s:%(lineno)d - %(message)s"
        )

        # Add a stream handler to log to console
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setFormatter(formatter)
        self.addHandler(console_handler)

        # If log_file is provided, add a file handler to log to file
        if log_file:
            file_handler = logging.FileHandler(log_file)
            file_handler.setFormatter(formatter)
            self.addHandler(file_handler)
