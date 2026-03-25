# Copyright 2023 Dexory (c)


def pytest_configure(config):
    config.addinivalue_line(
        'markers',
        'ament_black: marks tests checking for ament-black CLI interface',
    )
