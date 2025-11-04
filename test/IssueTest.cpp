#include <gtest/gtest.h>
#include "Issue.hpp"

// ----- Happy path basics -----
TEST(Issue, NewStartsWithIdZeroAndNoAssigneeOrDescription) {
  Issue is{0, "u1", "Crash on save"};
  EXPECT_EQ(is.getId(), 0);
  EXPECT_EQ(is.getAuthorId(), "u1");
  EXPECT_EQ(is.getTitle(), "Crash on save");
  EXPECT_FALSE(is.hasPersistentId());
  EXPECT_FALSE(is.hasAssignee());
  EXPECT_FALSE(is.hasDescriptionComment());
  EXPECT_TRUE(is.getCommentIds().empty());
}

// ----- ID contract: repo assigns once -----
TEST(Issue, RepoAssignsIdOnce) {
  Issue is{0, "u1", "Assign id"};
  EXPECT_FALSE(is.hasPersistentId());

  is.setIdForPersistence(101);
  EXPECT_TRUE(is.hasPersistentId());
  EXPECT_EQ(is.getId(), 101);

  // Calling again should throw
  EXPECT_THROW(is.setIdForPersistence(202), std::logic_error);
}

// Non-positive id assignment is invalid
TEST(Issue, RepoAssignRejectsNonPositiveId) {
  Issue is{0, "u1", "X"};
  EXPECT_THROW(is.setIdForPersistence(0), std::invalid_argument);
  EXPECT_THROW(is.setIdForPersistence(-3), std::invalid_argument);
}

// ----- Title validation -----
TEST(Issue, TitleValidation) {
  Issue is{0, "u1", "t"};
  is.setTitle("okay");
  EXPECT_EQ(is.getTitle(), "okay");
  EXPECT_THROW(is.setTitle(""), std::invalid_argument);
}

// ----- Comment list + description behavior -----
TEST(Issue, CommentsAddDedupRemoveAndDescription) {
  Issue is{0, "u1", "comments"};

  // add & dedup
  is.addComment(10);
  is.addComment(10);
  ASSERT_EQ(is.getCommentIds().size(), 1u);
  EXPECT_EQ(is.getCommentIds().front(), 10);

  // set description (auto-tracks in list)
  is.setDescriptionCommentId(10);
  EXPECT_TRUE(is.hasDescriptionComment());
  EXPECT_EQ(is.getDescriptionCommentId(), 10);

  // removing description clears it
  EXPECT_TRUE(is.removeComment(10));
  EXPECT_FALSE(is.hasDescriptionComment());

  // removing a non-existent comment returns false
  EXPECT_FALSE(is.removeComment(10));
}

// Invalid comment ids rejected
TEST(Issue, CommentIdValidation) {
  Issue is{0, "u1", "bad ids"};
  EXPECT_THROW(is.addComment(0), std::invalid_argument);
  EXPECT_THROW(is.addComment(-5), std::invalid_argument);
  EXPECT_THROW(is.setDescriptionCommentId(0), std::invalid_argument);
  EXPECT_THROW(is.setDescriptionCommentId(-9), std::invalid_argument);
}

// ----- Assignee toggle -----
TEST(Issue, AssignAndUnassign) {
  Issue is{0, "u1", "assignee"};
  EXPECT_FALSE(is.hasAssignee());

  is.assignTo("u9");
  ASSERT_TRUE(is.hasAssignee());
  EXPECT_EQ(is.getAssignedTo(), "u9");

  is.unassign();
  EXPECT_FALSE(is.hasAssignee());
}

// ----- Ctor validation -----
TEST(Issue, CtorRejectsNegativeId) {
  EXPECT_THROW((Issue{-1, "u1", "t"}), std::invalid_argument);
}

TEST(Issue, CtorRejectsEmptyAuthor) {
  EXPECT_THROW((Issue{0, "", "t"}), std::invalid_argument);
}

TEST(Issue, CtorRejectsEmptyTitle) {
  EXPECT_THROW((Issue{0, "u1", ""}), std::invalid_argument);
}
