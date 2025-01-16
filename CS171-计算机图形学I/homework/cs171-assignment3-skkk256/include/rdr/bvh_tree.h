// You might wonder why we need this while having an existing bvh_accel
// The abstraction level of these two are different. bvh_tree as a generic class
// can be used to implement bvh_accel, but bvh_accel itself can only encapsulate
// TriangleMesh thus cannot be used with bvhtree.
#ifndef __BVH_TREE_H__
#define __BVH_TREE_H__

#include "rdr/accel.h"
#include "rdr/primitive.h"
#include "rdr/ray.h"

RDR_NAMESPACE_BEGIN

template <typename DataType_>
class BVHNodeInterface {
public:
  using DataType = DataType_;

  // The only two required interfaces
  virtual AABB getAABB() const            = 0;
  virtual const DataType &getData() const = 0;

protected:
  // Interface spec
  BVHNodeInterface()  = default;
  ~BVHNodeInterface() = default;

  BVHNodeInterface(const BVHNodeInterface &)            = default;
  BVHNodeInterface &operator=(const BVHNodeInterface &) = default;
};

template <typename NodeType_>
class BVHTree final {
public:
  using NodeType  = NodeType_;
  using IndexType = int;

  // Context-local
  constexpr static int INVALID_INDEX = -1;
  constexpr static int CUTOFF_DEPTH  = 22;

  enum class EHeuristicProfile {
    EMedianHeuristic      = 0,  ///<! use centroid[dim]
    ESurfaceAreaHeuristic = 1,  ///<! use SAH (see PBRT)
  };

  // The actual node that represents the tree structure
  struct InternalNode {
    InternalNode() = default;
    InternalNode(IndexType span_left, IndexType span_right)
        : span_left(span_left), span_right(span_right) {}

    bool is_leaf{false};
    IndexType left_index{INVALID_INDEX};
    IndexType right_index{INVALID_INDEX};
    IndexType span_left{INVALID_INDEX};
    IndexType span_right{INVALID_INDEX};  // nodes[span_left, span_right)
    AABB aabb{};                          // The bounding box of the node
  };

  BVHTree()  = default;
  ~BVHTree() = default;
  /// General Interface
  size_t size() { return nodes.size(); }

  /// Nodes might be re-ordered
  void push_back(const NodeType &node) { nodes.push_back(node); }
  const AABB &getAABB() const { return internal_nodes[root_index].aabb; }

  /// reset build status
  void clear();

  /// *Can* be executed not only once
  void build();

  template <typename Callback>
  bool intersect(Ray &ray, Callback callback) const {
    if (!is_built) return false;
    return intersect(ray, root_index, callback);
  }

private:
  EHeuristicProfile hprofile{EHeuristicProfile::EMedianHeuristic};
//  EHeuristicProfile hprofile{EHeuristicProfile::ESurfaceAreaHeuristic};


  bool is_built{false};
  IndexType root_index{INVALID_INDEX};

  vector<NodeType> nodes{};               /// The data nodes
  vector<InternalNode> internal_nodes{};  /// The internal nodes

  /// Internal build
  IndexType build(
      int depth, const IndexType &span_left, const IndexType &span_right);

  /// Internal intersect
  template <typename Callback>
  bool intersect(
      Ray &ray, const IndexType &node_index, Callback callback) const;
};

/* ===================================================================== *
 *
 * Implementation
 *
 * ===================================================================== */

template <typename NodeType>
void BVHTree<NodeType>::clear() {
  nodes.clear();
  internal_nodes.clear();
  is_built = false;
}

template <typename NodeType>
void BVHTree<NodeType>::build() {
  if (is_built) return;

  // pre-allocate memory
  internal_nodes.reserve(2 * nodes.size());
  root_index = build(0, 0, nodes.size());
  is_built   = true;
}

template <typename NodeType>
typename BVHTree<NodeType>::IndexType BVHTree<NodeType>::build(
    int depth, const IndexType &span_left, const IndexType &span_right) {
  if (span_left >= span_right) return INVALID_INDEX;

  // early calculate bound
  AABB prebuilt_aabb;
  for (IndexType span_index = span_left; span_index < span_right; ++span_index)
    prebuilt_aabb.unionWith(nodes[span_index].getAABB());

  if (depth >= CUTOFF_DEPTH || span_left + 1 == span_right) {
    // Leaf node
    const auto &node = nodes[span_left];
    InternalNode result(span_left, span_right);
    result.is_leaf = true;
    result.aabb    = prebuilt_aabb;
    internal_nodes.push_back(result);
    return internal_nodes.size() - 1;
  }

  // You'll notice that the implementation here is different from the KD-Tree
  // ones, which re-use the node for both data-storing and organizing the real
  // tree structure. Here, for simplicity and generality, we use two different
  // types of nodes to ensure simplicity in interface, i.e. provided node does
  // not need to be aware of the tree structure.
  InternalNode result(span_left, span_right);

//#if 0
//  // Or probably...?
//  const int &dim = depth % 3;
//#endif
  const int &dim  = ArgMax(prebuilt_aabb.getExtent());
  IndexType count = span_right - span_left;
  IndexType split = INVALID_INDEX;

  if (hprofile == EHeuristicProfile::EMedianHeuristic) {
use_median_heuristic:
    // TODO: fill in your implementation here.
    // 1. Calculate the split point with `split` and `count`.
    // 2. Use `std::nth_element` to "sort" `this->nodes` with a criterion such
    // that their AABB center (obtained possibly by
    // `NodeType.getAABB().getCenter()[dim]`?).
    // 3. Export the `split` variable for later calculation
    std::nth_element(nodes.begin() + span_left, nodes.begin() + span_left + count / 2, nodes.begin() + span_right,
                     [dim](const NodeType &a, const NodeType &b) {
                       return a.getAABB().getCenter()[dim] < b.getAABB().getCenter()[dim];
                     });
    split = span_left + count / 2;
  } else if (hprofile == EHeuristicProfile::ESurfaceAreaHeuristic) {
    use_surface_area_heuristic:
    // TODO(bonus): fill in your implementation here.
    // Implement the surface area heuristic. The result might be surprisingly
    // good.
    std::vector<AABB> left_aabbs(count), right_aabbs(count);
    split = span_left;
    Float min_cost = Float_INF;

    // Sort nodes by the center of AABB in the selected dimension
    std::sort(nodes.begin() + span_left, nodes.begin() + span_right,
              [dim](const NodeType &a, const NodeType &b) {
                return a.getAABB().getCenter()[dim] < b.getAABB().getCenter()[dim];
              });

//    AABB total;
//    for (int i = 0; i < count; ++i) {
//      if (i == 0) {
//        total = nodes[span_left+i].getAABB();
//      }
//      else{
//        total.unionWith(nodes[span_left+i].getAABB());
//      }
//    }
//    Float SN = total.getSurfaceArea();

    for (int i = 1; i < count; ++i) {
      auto beginning = nodes.begin() + span_left;
      auto middling = nodes.begin() + span_left + i;
      auto ending = nodes.begin() + span_right;
      auto leftshapes = std::vector<NodeType>(beginning, middling);
      auto rightshapes = std::vector<NodeType>(middling, ending);
//      Info_("leftshapes size: {}, rightshapes size: {}", leftshapes.size(), rightshapes.size());

      AABB left_aabb = leftshapes[0].getAABB();
      AABB right_aabb =  rightshapes[0].getAABB();
      for (int k = 1; k < leftshapes.size(); ++k)
        left_aabb.unionWith(leftshapes[k].getAABB());
      for (int k = 1; k < rightshapes.size(); ++k)
        right_aabb.unionWith(rightshapes[k].getAABB());
      float SA = left_aabb.getSurfaceArea(); //SA
      float SB = right_aabb.getSurfaceArea(); //SB
      float cost = leftshapes.size() * SA + rightshapes.size() * SB;
//      Info_("cost: {}, min_cost: {}", cost, min_cost);
      if (cost < min_cost)
      {
        min_cost = cost;
        split = i + span_left;
      }
    }
//    Info_("split: {}", split);
//    Info_("------------------------");
  }
  // Build the left and right subtree
  result.left_index  = build(depth + 1, span_left, split);
  result.right_index = build(depth + 1, split, span_right);

  // Iterative merge
  result.aabb = prebuilt_aabb;

  internal_nodes.push_back(result);
  return internal_nodes.size() - 1;
}

template <typename NodeType>
template <typename Callback>
bool BVHTree<NodeType>::intersect(
    Ray &ray, const IndexType &node_index, Callback callback) const {
  bool result              = false;
  const InternalNode &node = internal_nodes[node_index];

  // Perform the actual pruning
  Float t_in  = NAN;
  Float t_out = NAN;
  if (!node.aabb.intersect(ray, &t_in, &t_out)) return result;

  if (node.is_leaf) {
    // TODO: fill in your implementation here.
    // The callback function has the following signature: bool(Ray&, const
    // DataType&). Accepting a reference to the ray(because ray.tMax it to be
    // modified and considered along the way) and the data stored in the node.
    // The callback will return a boolean value indicating whether the ray(with
    // tMin and tMax considered) is intersecting with the data.
    //
    // You should invoke the
    // 1. Iterate over the data in the span and call the callback function on
    // `this->node`.
    // 2. Return the result (intersect or not).
    for (IndexType i = node.span_left; i < node.span_right; i++) {
      if (callback(ray, this->nodes[i].getData())) {
        result = true;
      }
    }
  } else {
    // TODO: fill in your implementation here.
    // 1. Recurse to the left and right subtree.
    // 2. Return the result (intersect or not).
    bool left = intersect(ray, node.left_index, callback);
    bool right = intersect(ray, node.right_index, callback);
    result = left || right;
  }
  return result;
}

RDR_NAMESPACE_END

#endif
