-- CampusNode LMS database schema (SQLite)
-- The program must run "PRAGMA foreign_keys = ON;" on every new connection.
-- Dates and times are stored as ISO-8601 text: 'YYYY-MM-DD HH:MM:SS'.

PRAGMA foreign_keys = ON;

CREATE TABLE department (
    department_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    name           TEXT NOT NULL UNIQUE
);

CREATE TABLE users (
    user_id         INTEGER PRIMARY KEY AUTOINCREMENT,
    username        TEXT NOT NULL UNIQUE,
    full_name       TEXT NOT NULL,
    password_hash   TEXT NOT NULL,   -- salted hash (the salt is stored inside the hash string)
    role            TEXT NOT NULL CHECK (role IN ('ADMIN', 'LECTURER', 'HOD', 'STUDENT')),
    student_number  TEXT UNIQUE,     -- students only
    department_id   INTEGER REFERENCES department (department_id),  -- Heads of Department only
    active          INTEGER NOT NULL DEFAULT 1 CHECK (active IN (0, 1)),
    created_at      TEXT NOT NULL DEFAULT (datetime('now')),
    CHECK ((role = 'STUDENT') = (student_number IS NOT NULL)),
    CHECK ((role = 'HOD') = (department_id IS NOT NULL))
);

-- At most one Head of Department per department.
CREATE UNIQUE INDEX idx_one_hod_per_department ON users (department_id) WHERE role = 'HOD';

CREATE TABLE course (
    course_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    code           TEXT NOT NULL UNIQUE,
    title          TEXT NOT NULL,
    department_id  INTEGER NOT NULL REFERENCES department (department_id)
);
CREATE INDEX idx_course_department ON course (department_id);

CREATE TABLE course_lecturer (
    course_id    INTEGER NOT NULL REFERENCES course (course_id),
    lecturer_id  INTEGER NOT NULL REFERENCES users (user_id),
    PRIMARY KEY (course_id, lecturer_id)
);

CREATE TABLE enrolment (
    student_id   INTEGER NOT NULL REFERENCES users (user_id),
    course_id    INTEGER NOT NULL REFERENCES course (course_id),
    enrolled_on  TEXT NOT NULL DEFAULT (date('now')),
    PRIMARY KEY (student_id, course_id)
);
CREATE INDEX idx_enrolment_course ON enrolment (course_id);

CREATE TABLE assignment (
    assignment_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id      INTEGER NOT NULL REFERENCES course (course_id),
    title          TEXT NOT NULL,
    description    TEXT,
    deadline       TEXT NOT NULL,
    max_mark       REAL NOT NULL CHECK (max_mark > 0)
);
CREATE INDEX idx_assignment_course ON assignment (course_id);

CREATE TABLE extension (
    extension_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id  INTEGER NOT NULL REFERENCES assignment (assignment_id),
    student_id     INTEGER NOT NULL REFERENCES users (user_id),
    granted_by     INTEGER NOT NULL REFERENCES users (user_id),
    new_deadline   TEXT NOT NULL,
    reason         TEXT NOT NULL CHECK (length(trim(reason)) > 0),
    granted_at     TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX idx_extension_lookup ON extension (assignment_id, student_id);

CREATE TABLE submission (
    submission_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id  INTEGER NOT NULL REFERENCES assignment (assignment_id),
    student_id     INTEGER NOT NULL REFERENCES users (user_id),
    status         TEXT NOT NULL CHECK (status IN ('SUBMITTED', 'LATE', 'MARKED', 'MODERATED', 'RELEASED')),
    raw_mark       REAL CHECK (raw_mark IS NULL OR raw_mark >= 0),
    penalty        REAL NOT NULL DEFAULT 0 CHECK (penalty >= 0),
    final_mark     REAL CHECK (final_mark IS NULL OR final_mark >= 0),
    feedback       TEXT,
    UNIQUE (assignment_id, student_id),
    -- A submission has marks exactly when it is past the SUBMITTED / LATE states.
    CHECK ((status IN ('SUBMITTED', 'LATE')) = (raw_mark IS NULL AND final_mark IS NULL))
);
CREATE INDEX idx_submission_student ON submission (student_id);
CREATE INDEX idx_submission_status ON submission (status);

CREATE TABLE submission_version (
    version_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    submission_id  INTEGER NOT NULL REFERENCES submission (submission_id),
    version_no     INTEGER NOT NULL CHECK (version_no >= 1),
    content        TEXT NOT NULL CHECK (length(content) > 0),
    submitted_at   TEXT NOT NULL,
    UNIQUE (submission_id, version_no)
);

CREATE TABLE audit_entry (
    audit_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    submission_id  INTEGER NOT NULL REFERENCES submission (submission_id),
    changed_by     INTEGER NOT NULL REFERENCES users (user_id),
    changed_at     TEXT NOT NULL DEFAULT (datetime('now')),
    old_value      REAL,             -- NULL for the first mark
    new_value      REAL NOT NULL,
    reason         TEXT NOT NULL CHECK (length(trim(reason)) > 0)
);
CREATE INDEX idx_audit_submission ON audit_entry (submission_id);
CREATE INDEX idx_audit_changed_at ON audit_entry (changed_at);

-- The audit log is append-only: entries can never be changed or removed.
CREATE TRIGGER audit_entry_no_update BEFORE UPDATE ON audit_entry
BEGIN SELECT RAISE(ABORT, 'audit log is append-only'); END;

CREATE TRIGGER audit_entry_no_delete BEFORE DELETE ON audit_entry
BEGIN SELECT RAISE(ABORT, 'audit log is append-only'); END;
