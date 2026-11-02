// Composition root: builds every object once and starts the console program.
#include <iostream>
#include <memory>
#include <string>

#include "common/Exceptions.h"
#include "data/DatabaseManager.h"
#include "data/SqliteRepositories.h"
#include "services/AccessControl.h"
#include "services/LatePenaltyPolicy.h"
#include "services/SessionManager.h"
#include "ui/Console.h"
#include "ui/InputValidator.h"
#include "ui/Menus.h"

using namespace lms;

namespace {

void firstRunSetup(AdminService& admin) {
    console::heading("First run: create the administrator account");
    const std::string username = input::text("Username: ", 30);
    const std::string name = input::text("Full name: ", 100);
    const std::string password = console::readSecret("Password (at least 8 characters): ");
    if (password != console::readSecret("Repeat the password: ")) throw ValidationException("The two passwords are different.");
    admin.createFirstAdministrator(username, name, password);
    console::print("Administrator created. You can now log in.");
}

void runSession(Services& services, SessionManager& session) {
    while (true) {
        console::heading("CampusNode LMS: log in");
        const std::string username = console::readLine("Username (empty to quit): ");
        if (username.empty()) return;
        try {
            session.start(services.auth.login(username, console::readSecret("Password: ")));
        } catch (const AppException& e) {
            console::error(e.what());
            continue;
        }
        const User& user = session.user();
        switch (user.getRole()) {
            case Role::Admin: AdminMenu(user, services).run(); break;
            case Role::Lecturer: LecturerMenu(user, services).run(); break;
            case Role::Hod: HodMenu(static_cast<const HeadOfDepartment&>(user), services).run(); break;
            case Role::Student: StudentMenu(user, services).run(); break;
        }
        session.end();
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::string dbPath = "lms.db";
    for (int i = 1; i + 1 < argc; ++i) if (std::string(argv[i]) == "--db") dbPath = argv[i + 1];
    try {
        DatabaseManager db;
        db.open(dbPath);

        SqliteUserRepository users(db);
        SqliteCourseRepository courses(db);
        SqliteAssignmentRepository assignments(db);
        SqliteSubmissionRepository submissions(db);
        SqliteAuditRepository auditRepo(db);

        PasswordHasher hasher;
        AccessControl access;
        TenPercentPerDayPolicy latePolicy;
        SubmissionDetails details(users, assignments, courses);
        AuditService audit(auditRepo);
        AuthService auth(users, hasher);
        AdminService admin(users, courses, hasher, access, db);
        AssignmentService assignmentService(assignments, courses, users, submissions, access, db);
        SubmissionService submissionService(submissions, assignments, courses, access, db);
        MarkingService marking(submissions, assignments, courses, latePolicy, audit, details, access, db);
        ModerationService moderation(submissions, assignments, courses, audit, details, access, db);
        ResultsService results(submissions, assignments, courses, users, details, access);
        Services services{auth, admin, assignmentService, submissionService, marking, moderation, results, audit};
        SessionManager session;

        if (!admin.hasUsers()) firstRunSetup(admin);
        runSession(services, session);
        console::print("Goodbye.");
    } catch (const console::InputClosed&) {
        std::cout << "\nGoodbye.\n";
    } catch (const AppException& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << '\n';
        return 2;
    }
    return 0;
}
