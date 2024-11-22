# Task: Debug Set

This repository contains a class implementing a balanced binary search tree that verifies the correct usage of its
methods.

## Requirements for the Search Tree

The semantics of the functions must match the behavior of `std::set` from the standard library.

In `set.h`, the computational complexity for each function is specified, except
for `set::begin`, `set::end`, `set::iterator::operator++`, and `set::iterator::operator--`. For these, each function
should operate in no more than `O(h)`, and the aggregate complexity of the
code `for (auto i = s.begin(); i != s.end(); ++i);` must be `O(n)`.

An empty `set` must never allocate dynamic memory.

Insertions must not invalidate any iterators. Deletions invalidate only iterators pointing to the removed elements,
while `end()` always remains valid.

## Validity Checks

The class detects incorrect usage of its operations and terminates the program upon detecting such usage. Examples of
incorrect usage include:

1. Dereferencing/incrementing/decrementing/`erase()` of an invalid iterator.
2. Dereferencing the `end()` iterator.
3. Incrementing `end()`.
4. Decrementing `begin()`.
5. Calling `erase()` on the `end()` iterator.

### General Rule

Operations that would lead to undefined behavior in a standard `set` are detected here.

If incorrect usage is detected, the program is terminated using the `abort()` function.

### Exception Safety

The exception safety guarantees for all operations are preserved as in a standard `set`. However, the guarantees
for `begin()`, `end()`, `swap()` of the `set`, as well as copy and assignment operations for iterators, are weakened
from `nothrow` to `strong`.
