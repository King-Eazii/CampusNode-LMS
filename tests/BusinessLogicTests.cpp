
#include <gtest/gtest.h>
#include <filesystem>

#include "data/DatabaseManager.h"
#include "data/SqliteRepositories.h"
#include "domain/User.h"

#include "services/LatePenaltyPolicy.h"
#include "common/Validation.h"
#include "common/Exceptions.h"

using namespace lms;

// LATE PENALTY TESTS

class LatePenaltyTest : public ::testing::Test {
protected:
    TenPercentPerDayPolicy policy;
    const double maxMark = 100.0;
    const std::string deadline = "2026-01-01 10:00:00";
};

TEST_F(LatePenaltyTest, OnTimeSubmissionHasNoPenalty) {
    EXPECT_DOUBLE_EQ(
        policy.calculatePenalty(maxMark, deadline, deadline), 0.0
    );
}

TEST_F(LatePenaltyTest, OneMinuteLateGetsTenPercentPenalty) {
    EXPECT_DOUBLE_EQ(
        policy.calculatePenalty(maxMark, "2026-01-01 10:01:00", deadline), 10.0
    );
}

TEST_F(LatePenaltyTest, OneDayLateGetsTenPercentPenalty) {
    EXPECT_DOUBLE_EQ(
        policy.calculatePenalty(maxMark, "2026-01-02 10:00:00", deadline), 10.0
    );
}

TEST_F(LatePenaltyTest, OneDayAndOneMinuteLateGetsTwentyPercentPenalty) {
    EXPECT_DOUBLE_EQ(
        policy.calculatePenalty(maxMark, "2026-01-02 10:01:00", deadline), 20.0
    );
}

TEST_F(LatePenaltyTest, TwoDaysLateGetsTwentyPercentPenalty) {
    EXPECT_DOUBLE_EQ(
        policy.calculatePenalty(maxMark, "2026-01-03 10:00:00", deadline), 20.0
    );
}

TEST_F(LatePenaltyTest, ThreeDaysLateGetsThirtyPercentPenalty) {
    EXPECT_DOUBLE_EQ(
        policy.calculatePenalty(maxMark, "2026-01-04 10:00:00", deadline), 30.0
    );
}

TEST_F(LatePenaltyTest, MoreThanThreeDaysLateLosesEntireMark) {
    EXPECT_DOUBLE_EQ(
        policy.calculatePenalty(maxMark, "2026-01-04 10:01:00", deadline), 100.0
    );
}

// MARK VALIDATION TESTS

TEST(ValidationTest, AcceptsZeroMark) {
    EXPECT_NO_THROW(validation::requireMark(0, 100));
}

TEST(ValidationTest, AcceptsMaximumMark) {
    EXPECT_NO_THROW(validation::requireMark(100, 100));
}

TEST(ValidationTest, RejectsNegativeMark) {
    EXPECT_THROW(validation::requireMark(-1, 100), ValidationException);
}

TEST(ValidationTest, RejectsMarkAboveMaximum) {
    EXPECT_THROW(validation::requireMark(101, 100), ValidationException);
}

// USERNAME VALIDATION TESTS

TEST(ValidationTest, AcceptsValidUsername) {
    EXPECT_NO_THROW(validation::requireUsername("student123"));
}

TEST(ValidationTest, RejectsUsernameBelowMinimumLength) {
    EXPECT_THROW(validation::requireUsername("ab"), ValidationException);
}

TEST(ValidationTest, RejectsUsernameAboveMaximumLength) {
    EXPECT_THROW(
        validation::requireUsername("abcdefghijklmnopqrstuvwxyz12345"),
        ValidationException
    );
}

// REQUIRED TEXT VALIDATION TESTS

TEST(ValidationTest, RejectsEmptyRequiredText) {
    EXPECT_THROW(
        validation::requireText("   ", "Title", 100),
        ValidationException
    );
}

TEST(ValidationTest, AcceptsValidRequiredText) {
    EXPECT_NO_THROW(
        validation::requireText("Assignment 1", "Title", 100)
    );
}

// SQLITE USER REPOSITORY INTEGRATION TESTS

class UserRepositoryIntegrationTest : public ::testing::Test {
protected:
    lms::DatabaseManager db;
    std::unique_ptr<lms::SqliteUserRepository> repository;
    std::filesystem::path databasePath;

    void SetUp() override {
        databasePath =
            std::filesystem::temp_directory_path() /
            "campusnode_integration_test.db";

        std::error_code error;
        std::filesystem::remove(databasePath, error);

        db.open(databasePath.string());

        repository =
            std::make_unique<lms::SqliteUserRepository>(db);
    }

    void TearDown() override {
        repository.reset();
        db.close();

        std::error_code error;
        std::filesystem::remove(databasePath, error);
    }
};

TEST_F(UserRepositoryIntegrationTest, SavesAndRetrievesStudent) {

    lms::UserData data;
    data.username = "integration_student";
    data.fullName = "Integration Test Student";
    data.passwordHash = "test_hash";
    data.role = lms::Role::Student;
    data.studentNumber = "20230001";
    data.active = true;

    lms::Student student(data);

    int savedId = repository->save(student);

    EXPECT_GT(savedId, 0);

    std::unique_ptr<lms::User> retrieved =
        repository->findByUsername("integration_student");

    ASSERT_NE(retrieved, nullptr);

    EXPECT_EQ(retrieved->id(), savedId);
    EXPECT_EQ(retrieved->username(), "integration_student");
    EXPECT_EQ(retrieved->fullName(), "Integration Test Student");
    EXPECT_EQ(retrieved->getRole(), lms::Role::Student);
    EXPECT_TRUE(retrieved->isActive());

    lms::Student* retrievedStudent =
        dynamic_cast<lms::Student*>(retrieved.get());

    ASSERT_NE(retrievedStudent, nullptr);

    EXPECT_EQ(retrievedStudent->studentNumber(), "20230001");
}

TEST_F(UserRepositoryIntegrationTest, ReturnsNullForUnknownUsername) {

    std::unique_ptr<lms::User> retrieved =
        repository->findByUsername("unknown_student");

    EXPECT_EQ(retrieved, nullptr);
}
