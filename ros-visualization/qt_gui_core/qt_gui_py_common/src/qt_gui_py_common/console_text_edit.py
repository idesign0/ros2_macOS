# Software License Agreement (BSD License)
#
# Copyright (c) 2012, Dorian Scholz
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import sys

from python_qt_binding.QtCore import Qt
from python_qt_binding.QtGui import QFont
from python_qt_binding.QtWidgets import QTextEdit


class ConsoleTextEdit(QTextEdit):
    _color_stdout = Qt.blue
    _color_stderr = Qt.red
    _color_stdin = Qt.black
    _multi_line_char = '\\'
    _multi_line_indent = '    '
    _prompt = ('$ ', '  ')  # prompt for single and multi line

    class TextEditColoredWriter:

        def __init__(self, text_edit, color):
            self._text_edit = text_edit
            self._color = color

        def write(self, line):
            old_color = self._text_edit.textColor()
            self._text_edit.setTextColor(self._color)
            self._text_edit.insertPlainText(line)
            self._text_edit.setTextColor(old_color)
            self._text_edit.ensureCursorVisible()

    def __init__(self, parent=None):
        super(ConsoleTextEdit, self).__init__(parent)
        self.setFont(QFont('Mono'))

        self._multi_line = False
        self._multi_line_level = 0
        self._command = ''
        self._history = []
        self._history_index = -1

        # init colored writers
        self._stdout = self.TextEditColoredWriter(self, self._color_stdout)
        self._stderr = self.TextEditColoredWriter(self, self._color_stderr)
        self._comment_writer = self.TextEditColoredWriter(self, self._color_stdin)

    def print_message(self, msg):
        self._clear_current_line(clear_prompt=True)
        self._comment_writer.write(msg + '\n')
        self._add_prompt()

    def _add_prompt(self):
        self._comment_writer.write(
            self._prompt[self._multi_line] + self._multi_line_indent * self._multi_line_level)

    def _clear_current_line(self, clear_prompt=False):
        # block being current row
        prompt_length = len(self._prompt[self._multi_line])
        if clear_prompt:
            prompt_length = 0
        length = len(self.document().lastBlock().text()[prompt_length:])
        if length == 0:
            return None
        else:
            # should have a better way of doing this but I can't find it
            for _ in range(length):
                self.textCursor().deletePreviousChar()
        return True

    def _move_in_history(self, delta):
        # used when using the arrow keys to scroll through _history
        self._clear_current_line()
        if -1 <= self._history_index + delta < len(self._history):
            self._history_index += delta
        if self._history_index >= 0:
            self.insertPlainText(self._history[self._history_index])
        return True

    def _exec_code(self, code):
        raise NotImplementedError

    def _exec_with_captured_output(self, code):
        old_out, old_err = sys.stdout, sys.stderr
        sys.stdout, sys.stderr = self._stdout, self._stderr
        self._exec_code(code)
        sys.stdout, sys.stderr = old_out, old_err

    def keyPressEvent(self, event):
        prompt_length = len(self._prompt[self._multi_line])
        block_length = self.document().lastBlock().length()
        document_length = self.document().characterCount()
        line_start = document_length - block_length
        prompt_position = line_start + prompt_length

        # only handle keys if cursor is in the last line
        if self.textCursor().position() >= prompt_position:
            if event.key() == Qt.Key_Down:
                if self._history_index == len(self._history):
                    self._history_index -= 1
                self._move_in_history(-1)
                return None

            if event.key() == Qt.Key_Up:
                self._move_in_history(1)
                return None

            if event.key() in [Qt.Key_Backspace]:
                # don't allow cursor to delete into prompt
                if (self.textCursor().positionInBlock() == prompt_length and
                        not self.textCursor().hasSelection()):
                    return None

            if event.key() in [Qt.Key_Return, Qt.Key_Enter]:
                # set cursor to end of line to avoid line splitting
                cursor = self.textCursor()
                cursor.setPosition(document_length - 1)
                self.setTextCursor(cursor)

                self._history_index = -1
                line = str(self.document().lastBlock().text())[
                    prompt_length:].rstrip()  # remove prompt and trailing spaces

                self.insertPlainText('\n')
                if len(line) > 0:
                    if line[-1] == self._multi_line_char:
                        self._multi_line = True
                        self._multi_line_level += 1
                    self._history.insert(0, line)

                    if self._multi_line:  # multi line command
                        self._command += line + '\n'

                    else:  # single line command
                        self._exec_with_captured_output(line)
                        self._command = ''

                else:  # new line was is empty

                    if self._multi_line:  # multi line done
                        self._exec_with_captured_output(self._command)
                        self._command = ''
                        self._multi_line = False
                        self._multi_line_level = 0

                self._add_prompt()
                return None

        # allow all other key events
        super(ConsoleTextEdit, self).keyPressEvent(event)

        # fix cursor position to be after the prompt, if the cursor is in the last line
        if line_start <= self.textCursor().position() < prompt_position:
            cursor = self.textCursor()
            cursor.setPosition(prompt_position)
            self.setTextCursor(cursor)
