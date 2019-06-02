#include <gtest/gtest.h>

struct SomeCustomType {
  std::string hello;
  int world;
};

template <typename T>
class TypedTest : public ::testing::Test {
 public:
  T value;
};

using TestTypes = ::testing::Types<char, int, SomeCustomType>;
TYPED_TEST_CASE(TypedTest, TestTypes);

TYPED_TEST(TypedTest, Hello) {
  printf("Testing something %lu\n", sizeof(this->value));
}

TYPED_TEST(TypedTest, World) {
  ASSERT_GT(sizeof(this->value), 0);
}

