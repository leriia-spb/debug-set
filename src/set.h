#pragma once

#include <cassert>
#include <iterator>
#include <random>
#include <vector>
std::mt19937 mt;

template <typename T>
class set {
private:
  class set_iterator;

  struct base_node {
    base_node* right;
    base_node* left;
    base_node* parent;
    std::vector<set_iterator*> iterators;

    base_node() : left(this), right(this), parent(this) {}

    base_node(base_node* l, base_node* r, base_node* p) : left(l), right(r), parent(p) {}

    virtual ~base_node() {
      for (auto it : iterators) {
        it->is_valid = false;
      }
    }

    friend void swap(base_node& lhs, base_node& rhs) noexcept {
      std::swap(lhs.left, rhs.left);
      std::swap(lhs.right, rhs.right);
      std::swap(lhs.parent, rhs.parent);
    }
  };

  struct node : base_node {
    T value;
    size_t key;

    node(const T& val) : base_node(nullptr, nullptr, nullptr), value(val), key(mt()) {}

    ~node() override = default;
  };

  class set_iterator {
  public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = const T&;
    using pointer = const T*;
    using iterator_category = std::bidirectional_iterator_tag;

  private:
    base_node* _node;
    bool is_valid;
    const set* owner;

    void vector_add() {
      if (is_valid) {
        _node->iterators.push_back(this);
      }
    }

    void vector_del() {
      if (is_valid) {
        auto it = std::find(_node->iterators.begin(), _node->iterators.end(), this);
        if (it != _node->iterators.end()) {
          _node->iterators.erase(it);
        }
      }
    }

    void change_node(base_node* new_node) {
      vector_del();
      _node = new_node;
      vector_add();
    }

    set_iterator(base_node* node, const set* host) : _node(node), is_valid(true), owner(host) {
      try {
        vector_add();
      } catch (...) {
        is_valid = false;
        throw;
      }
    }
    friend class set;

  public:
    set_iterator() : _node(nullptr), is_valid(false), owner(nullptr) {}

    set_iterator(const set_iterator& other) : _node(other._node), is_valid(other.is_valid), owner(other.owner) {
      try {
        vector_add();
      } catch (...) {
        is_valid = false;
        throw;
      }
    }

    set_iterator& operator=(const set_iterator& other) {
      if (this != &other) {
        vector_del();

        _node = other._node;
        is_valid = other.is_valid;
        owner = other.owner;

        try {
          vector_add();
        } catch (...) {
          is_valid = false;
          throw;
        }
      }
      return *this;
    }

    ~set_iterator() {
      vector_del();
    }

    reference operator*() const {
      assert(is_valid);
      assert(_node != _node->right);
      return static_cast<node*>(_node)->value;
    }

    pointer operator->() const {
      assert(is_valid);
      assert(_node != _node->right);
      return &(static_cast<node*>(_node)->value);
    }

    set_iterator& operator++() {
      assert(is_valid);
      assert(_node != _node->right);
      if (_node->right) {
        change_node(_node->right);
        while (_node->left != nullptr && _node->left != _node) {
          change_node(_node->left);
        }
      } else {
        base_node* parent = _node->parent;
        while (_node->parent != nullptr && _node == parent->right) {
          change_node(parent);
          parent = parent->parent;
        }
        change_node(parent);
      }
      return *this;
    }

    set_iterator& operator--() {
      assert(is_valid);
      if (_node->left != nullptr && _node->left != _node) {
        change_node(_node->left);
        while (_node->right != nullptr && _node->right != _node) {
          change_node(_node->right);
        }
      } else {
        base_node* parent = _node->parent;
        while (parent != nullptr && _node == parent->left && _node != parent->right) {
          change_node(parent);
          parent = parent->parent;
        }
        change_node(parent);
      }
      assert(_node != _node->right);
      return *this;
    }

    set_iterator operator++(int) {
      set_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    set_iterator operator--(int) {
      set_iterator tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator==(const set_iterator& other) const {
      assert(is_valid);
      assert(other.is_valid);
      assert(owner == other.owner);
      return _node == other._node;
    }

    bool operator!=(const set_iterator& other) const {
      assert(is_valid);
      assert(other.is_valid);
      assert(owner == other.owner);
      return _node != other._node;
    }

    friend void swap(set_iterator& left, set_iterator& right) {
      assert(left.is_valid);
      assert(right.is_valid);

      assert(left._node != right._node);
      assert(left._node != left._node->right);
      assert(right._node != right._node->right);

      base_node* left_node = left._node;
      base_node* right_node = right._node;

      std::swap(left._node, right._node);
      std::swap(left.owner, right.owner);

      for (auto it : left_node->iterators) {
        it->_node = left_node;
      }
      for (auto it : right_node->iterators) {
        it->_node = right_node;
      }
    }
  };

public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = set_iterator;
  using const_iterator = set_iterator;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
  // O(1) nothrow
  set() noexcept : _root(base_node()) {}

  // O(n) strong
  set(const set& other) : set() {
    for (auto t = other.begin(); t != other.end(); t++) {
      insert(*t);
    }
  }

  // O(n) strong
  set& operator=(const set& other) {
    if (this != &other) {
      set temp(other);
      swap(*this, temp);
    }
    return *this;
  }

  // O(n) nothrow
  ~set() noexcept {
    if (empty()) {
      return;
    }
    deleting(_root.left);
  }

  // O(n) nothrow
  void clear() noexcept {
    if (empty()) {
      return;
    }
    deleting(_root.left);
    _size = 0;
  }

  // O(1) nothrow
  size_t size() const noexcept {
    return _size;
  }

  // O(1) nothrow
  bool empty() const noexcept {
    return size() == 0;
  }

  // nothrow
  const_iterator begin() const {
    if (empty()) {
      return end();
    }
    return const_iterator(most_left(_root.left), this);
  }

  // nothrow
  const_iterator end() const {
    return const_iterator(const_cast<base_node*>(&_root), this);
  }

  // nothrow
  const_reverse_iterator rbegin() const {
    return reverse_iterator(end());
  }

  // nothrow
  const_reverse_iterator rend() const {
    return reverse_iterator(begin());
  }

  // O(h) strong
  std::pair<iterator, bool> insert(const T& value) {
    node* new_node = nullptr;
    iterator it;
    if (empty()) {
      try {
        new_node = new node(value);
        it = iterator(new_node, this);
      } catch (...) {
        delete new_node;
        throw;
      }
      _root.left = new_node;
      new_node->parent = &_root;
      _size++;
      return {iterator(new_node, this), true};
    }
    node* try_find = find(_root.left, value);
    if (try_find) {
      return {iterator(try_find, this), false};
    }
    base_node* left = nullptr;
    base_node* right = nullptr;

    try {
      new_node = new node(value);
      it = iterator(new_node, this);
    } catch (...) {
      delete new_node;
      throw;
    }

    split(_root.left, value, left, right);

    auto root = merge(merge(left, new_node), right);
    root->parent = &_root;
    _root.left = root;
    _size++;
    return {it, true};
  }

  // O(h) nothrow
  iterator erase(const_iterator pos) {
    assert(pos.is_valid);
    assert(pos.owner == this);
    assert(pos._node != &_root);

    _size--;
    auto this_node = pos._node;
    pos++;

    auto kids = merge(this_node->left, this_node->right);

    if (this_node == &_root) {
      kids->parent = &_root;
      _root.left = kids;
    } else {
      if (this_node->parent->left == this_node) {
        this_node->parent->left = kids;
      } else {
        this_node->parent->right = kids;
      }
      if (kids) {
        kids->parent = this_node->parent;
      }
    }

    delete this_node;
    return pos;
  }

  // O(h) strong
  size_t erase(const T& value) {
    if (empty()) {
      return 0;
    }
    auto ans = find(value);
    if (ans == end()) {
      return 0;
    }
    erase(ans);
    return 1;
  }

  // O(h) strong
  const_iterator lower_bound(const T& value) const {
    base_node* current = _root.left;
    const_iterator result = end();

    while (current && current != current->right) {
      node* current_node = static_cast<node*>(current);

      if (current_node->value >= value) {
        result = const_iterator(current, this);
        current = current->left;
      } else {
        current = current->right;
      }
    }
    return result;
  }

  // O(h) strong
  const_iterator upper_bound(const T& value) const {
    base_node* current = _root.left;
    const_iterator result = end();

    while (current && current != current->right) {
      node* current_node = static_cast<node*>(current);

      if (current_node->value > value) {
        result = const_iterator(current, this);
        current = current->left;
      } else {
        current = current->right;
      }
    }
    return result;
  }

  // O(h) strong
  const_iterator find(const T& value) const {
    if (empty()) {
      return end();
    }
    node* ans = find(_root.left, value);
    if (ans) {
      return const_iterator(ans, this);
    }
    return end();
  }

  // O(1) strong
  friend void swap(set& left, set& right) noexcept {
    base_node* left_root = left._root.left;
    base_node* right_root = right._root.left;
    std::swap(left._root, right._root);
    std::swap(left._size, right._size);
    left_root->parent = &right._root;
    right_root->parent = &left._root;
  }

private:
  base_node _root;
  std::size_t _size = 0;

  void split(base_node* t, const T& value, base_node*& left, base_node*& right) const {
    if (t == nullptr) {
      left = right = nullptr;
    } else {
      node* currentNode = static_cast<node*>(t);
      if (currentNode->value < value) {
        left = currentNode;
        split(currentNode->right, value, left->right, right);
        if (left->right) {
          left->right->parent = left;
        }
        left->parent = nullptr;
      } else {
        right = currentNode;
        split(currentNode->left, value, left, right->left);
        if (right->left) {
          right->left->parent = right;
        }
        right->parent = nullptr;
      }
    }
  }

  base_node* merge(base_node* left, base_node* right) const {
    if (left == nullptr || right == nullptr) {
      return left ? left : right;
    }

    node* leftNode = static_cast<node*>(left);
    node* rightNode = static_cast<node*>(right);

    if (leftNode->key > rightNode->key) {
      leftNode->right = merge(leftNode->right, right);
      if (leftNode->right) {
        leftNode->right->parent = leftNode;
      }
      return leftNode;
    } else {
      rightNode->left = merge(left, rightNode->left);
      if (rightNode->left) {
        rightNode->left->parent = rightNode;
      }
      return rightNode;
    }
  }

  node* find(base_node* t, const T& value) const {
    if (!t) {
      return nullptr;
    }
    node* current_node = static_cast<node*>(t);
    if (current_node->value == value) {
      return current_node;
    }
    if (value < current_node->value) {
      return find(current_node->left, value);
    } else {
      return find(current_node->right, value);
    }
  }

  static base_node* most_left(base_node* n_node) {
    auto curr = n_node;
    while (curr->left) {
      curr = curr->left;
    }
    return curr;
  }

  void deleting(base_node* t) {
    if (t == nullptr) {
      return;
    }
    deleting(t->left);
    deleting(t->right);
    delete t;
  }
};