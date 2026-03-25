// Copyright 2025 Fictionlab sp. z o.o.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "aruco_opencv/board_loader.hpp"

#include <numeric>
#include "yaml-cpp/yaml.h"

namespace aruco_opencv
{

static bool parse_board(const YAML::Node & desc, BoardDescription & out, std::string & err)
{
  try {
    out.name = desc["name"].as<std::string>();
    out.frame_at_center = desc["frame_at_center"].as<bool>();
    out.markers_x = desc["markers_x"].as<int>();
    out.markers_y = desc["markers_y"].as<int>();
    out.marker_size = desc["marker_size"].as<double>();
    out.separation = desc["separation"].as<double>();
    out.first_id = desc["first_id"].as<int>();
    return true;
  } catch (const YAML::Exception & e) {
    err = e.what();
    return false;
  }
}

static cv::Ptr<cv::aruco::Board> make_grid_board(
  const BoardDescription & bd,
  const cv::Ptr<cv::aruco::Dictionary> & dict)
{
#if CV_VERSION_MAJOR > 4 || (CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7)
  std::vector<int> ids(bd.markers_x * bd.markers_y);
  std::iota(ids.begin(), ids.end(), bd.first_id);
  cv::Ptr<cv::aruco::Board> board = cv::makePtr<cv::aruco::GridBoard>(
      cv::Size(bd.markers_x, bd.markers_y), bd.marker_size, bd.separation, *dict, ids);
#else
  cv::Ptr<cv::aruco::Board> board = cv::aruco::GridBoard::create(
      bd.markers_x, bd.markers_y, bd.marker_size, bd.separation, dict, bd.first_id);
#endif
  if (!bd.frame_at_center) {
    return board;
  }
  double offset_x = (bd.markers_x * (bd.marker_size + bd.separation) - bd.separation) / 2.0;
  double offset_y = (bd.markers_y * (bd.marker_size + bd.separation) - bd.separation) / 2.0;
#if CV_VERSION_MAJOR > 4 || (CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7)
  std::vector<std::vector<cv::Point3f>> obj_points(board->getObjPoints());
#else
  std::vector<std::vector<cv::Point3f>> obj_points(board->objPoints);
#endif
  for (auto & obj : obj_points) {
    for (auto & point : obj) {
      point.x -= offset_x;
      point.y -= offset_y;
    }
  }
#if CV_VERSION_MAJOR > 4 || (CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7)
  return cv::makePtr<cv::aruco::Board>(obj_points, *dict, board->getIds());
#else
  return cv::aruco::Board::create(obj_points, dict, board->ids);
#endif
}

bool BoardLoader::load_from_file(
  const std::string & path,
  const cv::Ptr<cv::aruco::Dictionary> & dictionary,
  std::vector<std::pair<std::string, cv::Ptr<cv::aruco::Board>>> & out_boards,
  std::string & error_message)
{
  out_boards.clear();
  YAML::Node descriptions;
  try {
    descriptions = YAML::LoadFile(path);
  } catch (const YAML::Exception & e) {
    error_message = std::string("Failed to load board descriptions: ") + e.what();
    return false;
  }

  if (!descriptions.IsSequence()) {
    error_message = "Failed to load board descriptions: root node is not a sequence";
    return false;
  }

  for (const YAML::Node & desc : descriptions) {
    BoardDescription bd;
    std::string perr;
    if (!parse_board(desc, bd, perr)) {
      error_message = std::string("Failed to parse board: ") + perr;
      return false;
    }
    auto board = make_grid_board(bd, dictionary);
    out_boards.push_back(std::make_pair(bd.name, board));
  }
  return true;
}

}  // namespace aruco_opencv
