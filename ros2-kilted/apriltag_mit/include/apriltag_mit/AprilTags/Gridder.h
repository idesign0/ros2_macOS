#ifndef APRILTAGS_GRIDDER_H_
#define APRILTAGS_GRIDDER_H_

#include <algorithm>
#include <iterator>
#include <vector>

#include "apriltag_mit/AprilTags/Segment.h"

namespace AprilTags {

//! A lookup table in 2D for implementing nearest neighbor.
template <class T>
class Gridder {
 private:
  Gridder(const Gridder&);             //!< don't call
  Gridder& operator=(const Gridder&);  //!< don't call

  struct Cell {
    T* object;
    Cell* next;

    Cell() : object(nullptr), next(nullptr) {}

    Cell(const Cell& c) : object(c.object), next(c.next) {}

    // Destructor
    ~Cell() { delete next; }

    Cell& operator=(const Cell& other) {
      if (this == &other) return *this;

      object = other.object;
      next = other.next;
      return *this;
    }
  };

  float pixels_per_cell_;
  int width_, height_;
  std::vector<std::vector<Cell*>> cells_;

 public:
  Gridder(float pixel_width, float pixel_height, float pixels_per_cell)
      : pixels_per_cell_(pixels_per_cell) {
    width_ = pixel_width / pixels_per_cell + 1;
    height_ = pixel_height / pixels_per_cell + 1;

    cells_ = std::vector<std::vector<Cell*>>(
        height_, std::vector<Cell*>(width_, (Cell*)nullptr));
  }

  // Destructor
  ~Gridder() {
    for (unsigned int i = 0; i < cells_.size(); i++) {
      for (unsigned int j = 0; j < cells_[i].size(); j++) {
        delete cells_[i][j];
      }
    }
  }

  void Add(float x, float y, T* object) {
    int xc = x / pixels_per_cell_;
    int yc = y / pixels_per_cell_;

    if (xc >= 0 && yc >= 0 && xc < width_ && yc < height_) {
      Cell* c = new Cell;
      c->object = object;
      c->next = cells_[yc][xc];
      cells_[yc][xc] = c;
    }
  }

  // iterator begin();
  // iterator end();

  //! Iterator for Segment class.
  class Iterator {
   public:
    Iterator(Gridder* grid, float x, float y, float range)
        : outer(grid), xc0(), xc1(), yc0(), yc1(), xc(), yc(), c(nullptr) {
      Init(x, y, range);
    }

    //! Initializes Iterator constructor
    void Init(float x, float y, float range) {
      const auto ppc = outer->pixels_per_cell_;
      const auto w = outer->width_;
      const auto h = outer->height_;

      xc0 = (x - range) / ppc;
      yc0 = (y - range) / ppc;

      xc1 = (x + range) / ppc;
      yc1 = (y + range) / ppc;

      xc0 = std::max(0, xc0);
      xc0 = std::min(w - 1, xc0);

      xc1 = std::max(0, xc1);
      xc1 = std::min(w - 1, xc1);

      yc0 = std::max(0, yc0);
      yc0 = std::min(h - 1, yc0);

      yc1 = std::max(0, yc1);
      yc1 = std::min(h - 1, yc1);

      xc = xc0;
      yc = yc0;

      c = outer->cells_[yc][xc];
    }

    Iterator(const Iterator& it)
        : outer(it.outer),
          xc0(it.xc0),
          xc1(it.xc1),
          yc0(it.yc0),
          yc1(it.yc1),
          xc(it.xc),
          yc(it.yc),
          c(it.c) {}

    Iterator& operator=(const Iterator& it) {
      outer = it.outer;
      xc0 = it.xc0;
      xc1 = it.xc1;
      yc0 = it.yc0;
      yc1 = it.yc1;
      xc = it.xc;
      yc = it.yc;
      c = it.c;
    }

    bool hasNext() {
      if (c == nullptr) findNext();
      return (c != nullptr);
    }

    T& next() {
      T* thisObj = c->object;
      findNext();
      return *thisObj;  // return Segment
    }

   private:
    void findNext() {
      if (c != nullptr) c = c->next;
      if (c != nullptr) return;

      xc++;
      while (true) {
        if (xc > xc1) {
          yc++;
          xc = xc0;
        }
        if (yc > yc1) break;

        c = outer->cells_[yc][xc];

        if (c != nullptr) break;
        xc++;
      }
    }

    Gridder* outer;
    int xc0, xc1, yc0, yc1;
    int xc, yc;
    Cell* c;
  };

  typedef Iterator iterator;
  iterator find(float x, float y, float range) {
    return Iterator(this, x, y, range);
  }
};

}  // namespace AprilTags

#endif  // APRILTAGS_GRIDDER_H_
