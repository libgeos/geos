## GEOS is a port of JTS

* The algorithms that form the core value of GEOS are developed in Java in the [JTS library](https://github.com/locationtech/jts/). C++ developers will find this annoying, but:

  * This is just history, JTS was written first and GEOS was a slavish port.
  * Being memory managed, JTS is an easier language to prototype in.
  * Having various visual tooling, JTS is an easier platform to debug spatial algorithms in.
  * Being Java, JTS has less language legacy than GEOS, which was originally ported when STL was not part of the standard, and therefore reflects a mix of styles and eras.

* Ideally, new algorithms are implemented in JTS and then ported to GEOS.
  * Sometimes GEOS gains new functionality; ideally this is backported to JTS
* Performance optimizations in GEOS can backport to JTS.
  * Short circuits, indexes, and other non-language optimizations should be ticketed in JTS when they are added to GEOS.

### Follow JTS as Much as Possible

* Don't rename things! It makes it harder to port updates and fixes.

  * Class names
  * Method names
  * Variable names
  * Class members
    * If class members are shadowed by local variables, use the `m_` prefix convention for the member.

## C/C++ Guidelines

C++ is a large, complex language, with many patterns that have evolved over the years.
The GEOS codebase has also evolved over the years, but parts still exhibit obsolete language
and project patterns.
When porting or adding code, follow the style of the most recently written code (use the commit history to find this).

In general, we follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).
The following summarizes some of the key patterns used in GEOS.

### Manage Lifecycles

* Frequently objects are only used local to a method and not returned to the caller.
* In such cases, avoid lifecycle issues entirely by **instantiating on the local stack**.

```java
MyObj foo = new MyObj("bar");
```

```c++
MyObj foo("bar");
```

* Long-lived members of objects that are passed around should be held using [std::unique_ptr<>](https://en.cppreference.com/w/cpp/memory/unique_ptr).

```java
private MyMember foo = new MyMember();
```

```c++
private:

   std::unique_ptr<MyMember> foo;

public:

    MyMember()
        : foo(new MyMember())
        {}
```

* You can pass pointers to the object to other methods using `std::unique_ptr<>.get()`.

### Resource Management
[Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r-resource-management).
* [A raw pointer (a T*) is non-owning](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-ptr)
* [A raw reference (a T&) is non-owning](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-ref)

### Function Parameter calling conventions

* Objects which are passed as read-only with no transfer of ownership
  should be passed as `const` [references](https://en.wikipedia.org/wiki/Reference_%28C++%29#) (e.g. `const Geometry&`).
  References are simpler and safer to use than pointers.
  * in particular, this applies to Geometry objects passed to API operations
  (in static functions, class constructors, or initialization methods)
  * NOTE: many GEOS API functions use pass-by-pointer (`Geometry*`), but this is historical baggage
  * for the common case of passing a `std::unique_ptr<Geometry> foo` the argument will be `*foo`
* Pass-by-pointer can be used if the argument may be NULL (this is rare for API functions)

### Avoid Many Small Heap Allocations

* Heap allocations (objects created using `new`) are more expensive than stack allocations, but they can show up in batches in JTS in places where structures are built, like index trees or graphs.
* To both lower the overhead of heap allocations, and to manage the life-cycle of the objects, we recommend storing small objects in an appropriate "double-ended queue", like [std::deque<>](https://en.cppreference.com/w/cpp/container/deque).
* The implementation of `edgegraph` is an example.

  * The `edgegraph` consists of a structure of many `HalfEdge` objects (two for each edge!), created in the `EdgeGraph::createEdge()` method and stored in a `std::deque<>`.
  * The `std::deque<>` provides two benefits:

    * It lowers the number of heap allocations, because it allocates larger blocks of space to store multiple `HalfEdge` objects.
    * It handles the lifecycle of the `HalfEdge` objects that make up the `EdgeGraph`, because when the `EdgeGraph` is deallocated, the `std::deque<>` and all its contents are also automatically deallocated.

### Use `pragma once` to limit header inclusion

Use `#pragma once` to limit header inclusion.  It is simpler and faster.

### Use forward declarations in header files

Where possible, in header files use **forward declarations** rather than header includes.
This cuts the include dependency chain, which reduces the recompilation required when a low-level class header changes.
Includes are only needed where the class contents are referred to, not just the class name.

E.g. use this
```
namespace geos {
namespace geom {
class Geometry;
}
}
```
rather than:
```
#include <geos/geom/Geometry.h>
```

### Use `using` to reduce namespace qualifiers

GEOS is organized into many nested namespaces to reflect the JTS package structure.
Namespace qualification of class references can be shortened
with the `using` keyword in two different ways:

In **header files**, `using` [type aliases](https://en.cppreference.com/w/cpp/language/type_alias) can be defined **inside class definitions**:

```
class OffsetCurve {
  using Geometry = geom::Geometry;
  ...
}
```

In **implementation files**, `using` [declarations](https://en.cppreference.com/w/cpp/language/using_declaration) can be defined for classes or entire namespaces:

```
using geos::algorithm::Distance;
using namespace geos::geom;
```
