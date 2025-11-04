#include <gtest/gtest.h>
#include "Comment.hpp"

TEST(Comment, ConstructNewWithIdZero) {
  Comment c{0, "u1", "hi"};
  EXPECT_EQ(c.getId(), 0);
  EXPECT_EQ(c.getAuthor(), "u1");
  EXPECT_EQ(c.getText(), "hi");
  EXPECT_EQ(c.getTimeStamp(), 0);
  //cover the fresh-object path of hasPersistentId()
  EXPECT_FALSE(c.hasPersistentId());
}

TEST(Comment, RepoAssignsIdOnce) {
  Comment c{0, "u1", "t"};
  EXPECT_FALSE(c.hasPersistentId());
  c.setIdForPersistence(5);
  EXPECT_TRUE(c.hasPersistentId());
  EXPECT_EQ(c.getId(), 5);
  //cover the “id already set” guard/throw
  EXPECT_THROW(c.setIdForPersistence(6), std::logic_error);
}

TEST(Comment, TextValidation) {
  Comment c{0, "u1", "ok"};
  c.setText("updated");
  EXPECT_EQ(c.getText(), "updated");
  EXPECT_THROW(c.setText(""), std::invalid_argument);
}

TEST(Comment, TimestampSet) {
  Comment c{0, "u1", "t", 0};
  c.setTimeStamp(123456789);
  EXPECT_EQ(c.getTimeStamp(), 123456789);
}
