#pragma once
#include <cstddef>
#include <vector>
namespace PortPlatformRestore {
template<class IsParent, class Apply>
bool ParentsFirst(std::size_t count, IsParent parent, Apply apply) {
  std::vector<unsigned char> mark(count, 0);
  std::vector<std::size_t> order;
  auto visit = [&](auto&& self, std::size_t child) -> bool {
    if (mark[child] == 2) return true;
    if (mark[child] == 1) return false;
    mark[child] = 1;
    for (std::size_t p = 0; p < count; ++p)
      if (parent(p, child) && !self(self, p)) return false;
    mark[child] = 2;
    order.push_back(child);
    return true;
  };
  for (std::size_t i = 0; i < count; ++i) if (!visit(visit, i)) return false;
  for (std::size_t i : order) apply(i);
  return true;
}
}
