"""ros_sugar launch tools"""

import launch
import logging


LOGGER_NAME = "Launcher"


# Logger colors
class _ANSIColors:
    RESET = "\033[0m"

    # Muted/Pastel colors (to differentiate from components logging colors)
    DEBUG = "\033[38;5;109m"  # Soft Blue/Cyan (#8abeb7)
    INFO = "\033[38;5;143m"  # Olive Green    (#b5bd68)
    WARN = "\033[38;5;215m"  # Soft Orange    (#de935f)
    ERROR = "\033[38;5;167m"  # Soft Red       (#cc6666)
    FATAL = "\033[38;5;139m"  # Soft Purple    (#b294bb)


# Logger Custom Formatter
class _ColoredFormatter(logging.Formatter):
    def format(self, record):
        # Select color based on log level
        if record.levelno == logging.DEBUG:
            color = _ANSIColors.DEBUG
        elif record.levelno == logging.INFO:
            color = _ANSIColors.INFO
        elif record.levelno == logging.WARNING:
            color = _ANSIColors.WARN
        elif record.levelno == logging.ERROR:
            color = _ANSIColors.ERROR
        elif record.levelno == logging.CRITICAL:
            color = _ANSIColors.ERROR
        else:
            color = _ANSIColors.RESET

        # Format the actual message: "[INFO] [logger_name]: Message"
        formatted_msg = super().format(record)
        return f"{color}{formatted_msg}{_ANSIColors.RESET}"


# Apply the Formatter to the Launch Logger
def _apply_colored_logging(root_logger):
    # Iterate through handlers and swap the formatter
    for handler in root_logger.handlers:
        handler.setFormatter(
            _ColoredFormatter("[%(levelname)s] [%(name)s]: %(message)s")
        )


logger = launch.logging.get_logger(LOGGER_NAME)

_apply_colored_logging(logger)

# Capture the original error method
_original_error_log = launch.logging.LaunchLogger.error


# Define replacement method to avoid verbose process exit loggings
def _patched_error_log(self, msg, *args, **kwargs):
    if "process has died" in str(msg) and "cmd '" in str(msg):
        # suppress on process long and verbose exit message (can be replace inside our launcher)
        return

    # Otherwise, pass everything else to the original logger
    _original_error_log(self, msg, *args, **kwargs)


# 3. Apply the patch: Replace the class method with our version
launch.logging.LaunchLogger.error = _patched_error_log

__all__ = ["logger"]
