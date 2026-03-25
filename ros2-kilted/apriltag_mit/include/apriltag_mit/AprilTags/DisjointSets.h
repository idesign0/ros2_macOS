#ifndef APRILTAGS_DISJOINTSETS_H_
#define APRILTAGS_DISJOINTSETS_H_

#include <vector>

namespace AprilTags {

class DisjointSets {
 public:
  explicit DisjointSets(int n);

  int GetSetSize(int id);

  int Find(int id);

  //! Returns the id of the merged node.
  int Union(int id0, int id1);

 private:
  std::vector<int> parent_;
  std::vector<int> size_;
};

}  // namespace AprilTags

#endif  // APRILTAGS_DISJOINTSETS_H_
