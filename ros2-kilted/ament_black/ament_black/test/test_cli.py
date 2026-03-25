# Copyright 2023 Dexory (c)

import os

import pytest


@pytest.mark.ament_black
@pytest.mark.linter
def test_ament_black():
    rc = os.system('ament_black --help')
    assert rc == 0, 'ament-black python package not properly installed'
