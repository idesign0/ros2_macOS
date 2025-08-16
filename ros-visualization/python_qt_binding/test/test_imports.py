# Copyright 2018, PickNik Consulting
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


def test_import_qtcore():
    from python_qt_binding import QtCore
    assert QtCore is not None


def test_import_qtgui():
    from python_qt_binding import QtGui
    assert QtGui is not None


def test_import_qtwidgets():
    from python_qt_binding import QtWidgets
    assert QtWidgets is not None


def test_import_qtobject():
    from python_qt_binding.QtCore import QObject
    assert QObject is not None
