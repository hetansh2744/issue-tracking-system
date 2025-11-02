#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockIssueRepository.h"
#include "IssueTrackerController.h"

using namespace testing;

// --- Test Fixture ---

/**
 * @brief Test fixture for IssueTrackerController.
 * * Provides a MockIssueRepository instance that can be used and verified in tests.
 */
class IssueTrackerControllerTest : public Test {
protected:
    MockIssueRepository mockRepo;
};

/**
 * @brief Test Case 1: Constructor (Dependency Injection Test)
 * * Verifies that the controller successfully accepts and stores the IssueRepository 
 * dependency via the constructor (repo(repository) in the initializer list).
 */
TEST_F(IssueTrackerControllerTest, ConstructorInitializesDependency) {
    // 1. Arrange: Define what the mock should do when saveIssue is called
    // We expect the mock to be called with any Issue object, and we tell it
    // to return a dummy Issue with a non-zero ID (1001) to simulate persistence.
    Issue createdIssue("Test Title", "Test Desc", "user1", 1001);

    EXPECT_CALL(mockRepo, saveIssue(Matcher<const Issue&>(_)))
        .WillOnce(Return(createdIssue));

    // 2. Act: Creates a controller, with injected repo
    IssueTrackerController controller(mockRepo);

    // 3. Assert: Call a controller method to prove the dependency is functional
    Issue result = controller.createIssue("Test Title", "Test Desc", "user1");
    
    // If the creation succeeds and returns the mocked ID, the injection worked.
    ASSERT_EQ(result.id, 1001);
}

/**
 * @brief Test Case 2: Create Issue - Success
 * * Verifies successful issue creation
 */
TEST_F(IssueTrackerControllerTest, CreateIssue_Success) {
    // 1. Arrange: Define the expected return from the repository
    Issue expectedIssue("New Issue", "Details", "assignedUser", 2001);
    
    // We expect the repo's saveIssue method to be called exactly once.
    EXPECT_CALL(mockRepo, saveIssue(Matcher<const Issue&>(_)))
        .WillOnce(Return(expectedIssue));

    IssueTrackerController controller(mockRepo);
    
    // 2. Act
    Issue result = controller.createIssue("New Issue", "Details", "assignedUser");

    // 3. Assert
    ASSERT_EQ(result.id, 2001) << "The returned issue should have the ID assigned by the repo.";
    ASSERT_EQ(result.title, "New Issue");
}
