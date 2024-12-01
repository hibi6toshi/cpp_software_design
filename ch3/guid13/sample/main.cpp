#include <array>
#include <cstddef>
#include <cstdlib>
#include <memory_resource>
#include <string>
#include <vector>

int main() {
  std::array<std::byte, 1000> raw; // Note: not initialized!(未初期化)

  std::pmr::monotonic_buffer_resource buffer{ raw.data(), raw.size(), std::pmr::null_memory_resource()};

  std::pmr::vector<std::pmr::string> strings{ &buffer };

  strings.emplace_back("Srging longer than what SSO can handle");
  strings.emplace_back("Another long string that goes beyond SSO");
  strings.emplace_back("A third long string that cannot be gandled by SSO");

  // ...
  
  return EXIT_SUCCESS;
}