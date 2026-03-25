# Copyright 2022 Open Source Robotics Foundation, Inc.
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

from ament_copyright import UNKNOWN_IDENTIFIER
from ament_copyright.parser import FileDescriptor
from ament_copyright.parser import get_comment_block
from ament_copyright.parser import get_multiline_comment_block
from ament_copyright.parser import scan_past_empty_lines
from ament_copyright.parser import search_copyright_information
from ament_copyright.parser import split_template


def test_search_copyright_information_incorrect_typo():
    """Test searching for copyright information with a typo in the copyright information."""
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        'CopyrightTypo 2020 Open Source Robotics Foundation, Inc.'
    )
    assert len(copyrights) == 0
    assert years_span == []
    assert name_span == []
    assert len(remaining_block) == 56


def test_search_copyright_information_repeated():
    """Test searching with repeated 'copyright' in the copyright information."""
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        '\\copyright Copyright 2020 Open Source Robotics Foundation, Inc.'
    )
    assert len(copyrights) == 1
    assert years_span == [(21, 25)]
    assert name_span == [(26, 63)]
    assert len(remaining_block) == 0


def test_search_copyright_information_multiple_holders():
    """Test searching multiple holders."""
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        """Copyright 2020 Open Source Robotics Foundation, Inc.
           Copyright (c) 2009, Willow Garage, Inc."""
    )
    assert len(copyrights) == 2
    assert years_span == [(10, 14), (26, 30)]
    assert name_span == [(15, 52), (32, 51)]
    assert len(remaining_block) == 0


def test_search_copyright_information_capitalization1():
    """
    Test searching for copyright information with capitalized copyright information.

    Word 'copyright': capitalized
    Abbreviation '(c)': absent
    """
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        '  Copyright 2020 Open Source Robotics Foundation, Inc.')
    assert len(copyrights) == 1
    assert copyrights[0].name == 'Open Source Robotics Foundation, Inc.'
    assert years_span == [(12, 16)]
    assert name_span == [(17, 54)]
    assert len(remaining_block) == 0


def test_search_copyright_information_capitalization2():
    """
    Test searching for copyright information with capitalized copyright information.

    Word 'copyright': capitalized
    Abbreviation '(c)': lowercase
    """
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        'Copyright (c) 2020 Open Source Robotics Foundation, Inc.')
    assert len(copyrights) == 1
    assert years_span == [(14, 18)]
    assert name_span == [(19, 56)]
    assert len(remaining_block) == 0


def test_search_copyright_information_capitalization3():
    """
    Test searching for copyright information with capitalized copyright information.

    Word 'copyright': capitalized
    Abbreviation '(c)': uppercase
    """
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        'Copyright (C) 2020 Open Source Robotics Foundation, Inc.')
    assert len(copyrights) == 1
    assert years_span == [(14, 18)]
    assert name_span == [(19, 56)]
    assert len(remaining_block) == 0


def test_search_copyright_information_lowercase1():
    """
    Test searching for copyright information with lowercase copyright information.

    Word 'copyright': lowercase
    Abbreviation '(c)': absent
    """
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        'copyright 2020 Open Source Robotics Foundation, Inc.')
    assert len(copyrights) == 1
    assert years_span == [(10, 14)]
    assert name_span == [(15, 52)]
    assert len(remaining_block) == 0


def test_search_copyright_information_lowercase2():
    """
    Test searching for copyright information with lowercase copyright information and abbreviation.

    Word 'copyright': lowercase
    Abbreviation '(c)': lowercase
    """
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        'copyright (c) 2020 Open Source Robotics Foundation, Inc.')
    assert len(copyrights) == 1
    assert years_span == [(14, 18)]
    assert name_span == [(19, 56)]
    assert len(remaining_block) == 0


def test_search_copyright_information_uppercase1():
    """
    Test searching for copyright information with uppercase copyright information.

    Word 'copyright': uppercase
    Abbreviation '(c)': absent
    """
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        'COPYRIGHT 2020 Open Source Robotics Foundation, Inc.')
    assert len(copyrights) == 1
    assert years_span == [(10, 14)]
    assert name_span == [(15, 52)]
    assert len(remaining_block) == 0


def test_search_copyright_information_uppercase2():
    """
    Test searching for copyright information with uppercase copyright information.

    Word 'copyright': uppercase
    Abbreviation '(c)': uppercase
    """
    copyrights, years_span, name_span, remaining_block = search_copyright_information(
        'COPYRIGHT (C) 2020 Open Source Robotics Foundation, Inc.')
    assert len(copyrights) == 1
    assert years_span == [(14, 18)]
    assert name_span == [(19, 56)]
    assert len(remaining_block) == 0


def test_split_template_no_split():
    """Test the split_template with content that does not get split."""
    content = '1 2 3'
    sections = split_template(content, 'A')
    assert len(sections) == 1


def test_split_template_single_split():
    """Test the split_template with content that gets split once."""
    content = '1 A 2'
    sections = split_template(content, 'A')
    assert len(sections) == 2


def test_split_template_multiple_splits():
    """Test the split_template method with content split many times by different separators."""
    content = '1 A 2 B 3 A 4 C 5 B 6'
    sections = split_template(content, ['A', 'B', 'C'])
    assert len(sections) == 6


def test_identify_license():
    """
    Test the identify_license method with content that matches the given header template.

    This is a simple test case because the
    template only has one placeholder at the beginning, so it does not
    need to be split multiple times for matching.
    """
    content = """
    {copyright}

    Use of this source code is governed by an MIT-style
    license that can be found in the LICENSE file or at
    https://opensource.org/licenses/MIT.
    """

    class TempLicense(object):
        pass
    temp_license = TempLicense()
    temp_license.file_headers = [content]
    dut = FileDescriptor(0, '/')
    dut.identify_license(content, 'file_headers', {'temp': temp_license})
    assert dut.license_identifier == 'temp'


def test_identify_license_failure():
    """
    Test the identify_license method with content that contains a typo.

    Matching is expected to fail in this test case.
    """
    content = """
    {copyright}

    Use of this source code is governed by an MIT-style
    license that can be found in the LICENSE file or at
    https://opensource.org/licenses/MIT.
    """

    class TempLicense(object):
        pass
    temp_license = TempLicense()
    temp_license.file_headers = [content]
    dut = FileDescriptor(0, '/')
    dut.identify_license(
        content.replace('LICENSE', 'TYPO'),
        'file_headers',
        {'temp': temp_license}
    )
    assert dut.license_identifier == UNKNOWN_IDENTIFIER


def test_identify_license_multiple_splits():
    """
    Test the identify_license method with content that must be split multiple times.

    In order to match the header template that contains
    placeholders in multiple locations.
    """
    header_template = """
    {copyright}
    ALL RIGHTS RESERVED


    THIS UNPUBLISHED WORK BY {copyright_holder} IS PROTECTED. IF
    PUBLICATION OF THE WORK SHOULD OCCUR, THE FOLLOWING NOTICE SHALL
    APPLY.

     "{copyright} ALL RIGHTS RESERVED."

    {copyright_holder} DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE.
    """
    content = header_template.replace(
        '{copyright}',
        'Copyright 2020 Open Source Robotics Foundation, Inc.'
    ).replace(
        '{copyright_holder}',
        'Open Source Robotics Foundation, Inc.'
    )

    class TempLicense(object):
        pass
    temp_license = TempLicense()
    temp_license.file_headers = [content]
    dut = FileDescriptor(0, '/')
    dut.identify_license(content, 'file_headers', {'temp': temp_license})
    assert dut.license_identifier == 'temp'


def test_get_comment_block_slashes():
    """Test parsing comment block with c-style comment forward slashes."""
    commented_content = """
// aaa
// bbb
// ccc

// Comment not part of the header
    """
    index = 0
    index = scan_past_empty_lines(commented_content, index)
    block, _ = get_comment_block(commented_content, index)
    assert block is not None
    assert block == '\n'.join(['aaa', 'bbb', 'ccc'])


def test_get_comment_block_slashes2():
    """Test parsing comment multiline block that is not at the start of the content."""
    commented_content = """
// aaa
// bbb
// ccc

///
/**
ddd
*/
    """
    index = 0
    index = scan_past_empty_lines(commented_content, index)
    block = get_multiline_comment_block(commented_content, index)
    assert block is not None
    assert block == 'ddd'


def test_get_comment_block_doxygen():
    """Test parsing comment block with doxygen-style comment forward slashes."""
    commented_content = """
/// aaa
/// bbb
/// ccc
    """
    index = 0
    index = scan_past_empty_lines(commented_content, index)
    block, _ = get_comment_block(commented_content, index)
    assert block is not None
    assert block == '\n'.join(['aaa', 'bbb', 'ccc'])


def test_get_comment_block_pound():
    """Test parsing comment block with python-style comment pound signs."""
    commented_content = """
# aaa
# bbb
# ccc
    """
    index = 0
    index = scan_past_empty_lines(commented_content, index)
    block, _ = get_comment_block(commented_content, index)
    assert block is not None
    assert block == '\n'.join(['aaa', 'bbb', 'ccc'])


def test_get_multiline_comment_block_cstyle():
    """Test parsing comment block with multiline c-style comment block."""
    commented_content = """
/**
 * aaa
 * bbb
 * ccc
 */


/**
 * Comment not part of the header
 */
    """
    index = 0
    index = scan_past_empty_lines(commented_content, index)
    block = get_multiline_comment_block(commented_content, index)
    assert block is not None
    assert block == '\n'.join(['aaa', 'bbb', 'ccc'])


def test_get_multiline_comment_block_cstyle2():
    """Test parsing comment block with multiline c-style comment block."""
    commented_content = """
/**
 * aaa
 * bbb
 * ccc
 */

// Comment not part of
// the header
    """
    index = 0
    index = scan_past_empty_lines(commented_content, index)
    block = get_multiline_comment_block(commented_content, index)
    assert block is not None
    assert block == '\n'.join(['aaa', 'bbb', 'ccc'])


def test_get_multiline_comment_block_xmlstyle():
    """Test parsing comment block with multiline xml-style comment block."""
    commented_content = """
<!--
  aaa
  bbb
  ccc
 -->
    """
    index = 0
    index = scan_past_empty_lines(commented_content, index)
    block = get_multiline_comment_block(commented_content, index)
    assert block is not None
    assert block == '\n'.join(['aaa', 'bbb', 'ccc'])


def test_get_multiline_comment_block_xmlstyle_prefixed():
    """Test parsing comment block with multiline xml-style comment block containing a prefix."""
    commented_content = """
<!--
  # aaa
  # bbb
  # ccc
 -->
    """
    index = 0
    index = scan_past_empty_lines(commented_content, index)
    block = get_multiline_comment_block(commented_content, index)
    assert block is not None
    assert block == '\n'.join(['aaa', 'bbb', 'ccc'])
