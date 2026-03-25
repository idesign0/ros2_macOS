#include "apriltag_mit/AprilTags/DisjointSets.h"
#include <iostream>

namespace AprilTags {

DisjointSets::DisjointSets(int n) : parent_(n), size_(n, 1) {
  for (size_t i = 0; i < n; ++i) {
    // everyone is their own cluster of size 1
    parent_[i] = i;
  }
}

int DisjointSets::Find(int id) {
  // terminal case: a node is its own parent
  const int parent = parent_[id];
  if (parent == id)
    return id;

  // otherwise, recurse...
  int root = Find(parent);

  // short circuit the path
  parent_[id] = root;

  return root;
}

int DisjointSets::Union(int id0, int id1) {
  const int root0 = Find(id0);
  const int root1 = Find(id1);

  if (root0 == root1)
    return root0;

  const int sz0 = size_[root0];
  const int sz1 = size_[root1];

  if (sz0 > sz1) {
    parent_[root1] = root0;
    size_[root0] += sz1;
    return root0;
  } else {
    parent_[root0] = root1;
    size_[root1] += sz0;
    return root1;
  }
}

int DisjointSets::GetSetSize(int id) { return size_[Find(id)]; }

} // namespace AprilTags
