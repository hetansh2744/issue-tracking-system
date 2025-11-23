#include <gtest/gtest.h>
#include "Milestone.hpp"

// Constructor Tests
TEST(MilestoneTests, ConstructorValid) {
    Milestone milestone(1, "Milestone 1", "Description of milestone", "2023-01-01", "2023-12-31");
    EXPECT_EQ(milestone.getId(), 1);
    EXPECT_EQ(milestone.getName(), "Milestone 1");
    EXPECT_EQ(milestone.getDescription(), "Description of milestone");
    EXPECT_EQ(milestone.getStartDate(), "2023-01-01");
    EXPECT_EQ(milestone.getEndDate(), "2023-12-31");
}

TEST(MilestoneTests, ConstructorInvalidName) {
    EXPECT_THROW(Milestone milestone(1, "", "Description", "2023-01-01", "2023-12-31"), std::invalid_argument);
}

TEST(MilestoneTests, ConstructorInvalidStartDate) {
    EXPECT_THROW(Milestone milestone(1, "Milestone", "Description", "", "2023-12-31"), std::invalid_argument);
}

TEST(MilestoneTests, ConstructorInvalidEndDate) {
    EXPECT_THROW(Milestone milestone(1, "Milestone", "Description", "2023-01-01", ""), std::invalid_argument);
}

TEST(MilestoneTests, ConstructorNegativeID) {
    Milestone milestone(-1, "", "", "", "");
    EXPECT_EQ(milestone.getId(), -1);
}

// Setter Tests
TEST(MilestoneTests, SetNameValid) {
    Milestone milestone(1, "Initial", "Description", "2023-01-01", "2023-12-31");
    milestone.setName("Updated Name");
    EXPECT_EQ(milestone.getName(), "Updated Name");
}

TEST(MilestoneTests, SetNameInvalid) {
    Milestone milestone(1, "Initial", "Description", "2023-01-01", "2023-12-31");
    EXPECT_THROW(milestone.setName(""), std::invalid_argument);
}

TEST(MilestoneTests, SetDescription) {
    Milestone milestone(1, "Milestone", "Old Description", "2023-01-01", "2023-12-31");
    milestone.setDescription("New Description");
    EXPECT_EQ(milestone.getDescription(), "New Description");
}

TEST(MilestoneTests, SetStartDateValid) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    milestone.setDateStart("2024-01-01");
    EXPECT_EQ(milestone.getStartDate(), "2024-01-01");
}

TEST(MilestoneTests, SetStartDateInvalid) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    EXPECT_THROW(milestone.setDateStart(""), std::invalid_argument);
}

TEST(MilestoneTests, SetEndDateValid) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    milestone.setDateEnd("2024-01-01");
    EXPECT_EQ(milestone.getEndDate(), "2024-01-01");
}

TEST(MilestoneTests, SetEndDateInvalid) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    EXPECT_THROW(milestone.setDateEnd(""), std::invalid_argument);
}

// Issue Management Tests
TEST(MilestoneTests, AddIssue) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    milestone.addIssue(1001);
    EXPECT_EQ(milestone.getIssueCount(), 1);
    EXPECT_TRUE(milestone.hasIssue(1001));
}

TEST(MilestoneTests, AddDuplicateIssue) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    milestone.addIssue(1001);
    milestone.addIssue(1001);  // Duplicate
    EXPECT_EQ(milestone.getIssueCount(), 1);
}

TEST(MilestoneTests, RemoveIssue) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    milestone.addIssue(1001);
    milestone.removeIssue(1001);
    EXPECT_EQ(milestone.getIssueCount(), 0);
}

TEST(MilestoneTests, HasIssue) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    milestone.addIssue(1001);
    EXPECT_TRUE(milestone.hasIssue(1001));
    EXPECT_FALSE(milestone.hasIssue(1002));
}

TEST(MilestoneTests, GetIssueCount) {
    Milestone milestone(1, "Milestone", "Description", "2023-01-01", "2023-12-31");
    EXPECT_EQ(milestone.getIssueCount(), 0);
    milestone.addIssue(1001);
    EXPECT_EQ(milestone.getIssueCount(), 1);
}
