#include <catch_main.hpp>

#include "libslic3r/ProjectHistoryManager.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;

class TemporaryTree
{
public:
    TemporaryTree()
    {
        static std::atomic<unsigned long long> sequence{0};
        const auto                             stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = fs::temp_directory_path() / ("bambu-project-history-test-" + std::to_string(stamp) + "-" + std::to_string(sequence.fetch_add(1)));
        fs::create_directories(m_path);
    }

    ~TemporaryTree()
    {
        std::error_code error;
        fs::remove_all(m_path, error);
    }

    const fs::path &path() const { return m_path; }

private:
    fs::path m_path;
};

void write_binary(const fs::path &path, const std::vector<unsigned char> &bytes)
{
    fs::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    REQUIRE(output.good());
    output.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    REQUIRE(output.good());
}

std::vector<unsigned char> read_binary(const fs::path &path)
{
    std::ifstream input(path, std::ios::binary);
    REQUIRE(input.good());
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

Slic3r::ProjectHistoryCommitOptions commit_options(const std::string &message, std::int64_t unix_seconds)
{
    Slic3r::ProjectHistoryCommitOptions options;
    options.message      = message;
    options.author_name  = "Bambu Studio test";
    options.author_email = "project-history-test@localhost";
    options.committed_at = std::chrono::system_clock::time_point(std::chrono::seconds(unix_seconds));
    return options;
}

TEST_CASE("Project history stores complete snapshots in an isolated repository", "[project-history]")
{
    TemporaryTree                    temporary;
    const fs::path                   app_data     = temporary.path() / "app-data";
    const fs::path                   project      = temporary.path() / "user-projects" / fs::u8path(u8"project-\u6e2c\u8a66.3mf");
    const fs::path                   snapshot_one = temporary.path() / "completed" / "snapshot-one.3mf";
    const fs::path                   snapshot_two = temporary.path() / "completed" / "snapshot-two.3mf";
    const std::vector<unsigned char> bytes_one{'P', 'K', 3, 4, 0, 1, 2, 3, 0, 255};
    const std::vector<unsigned char> bytes_two{'P', 'K', 3, 4, 9, 8, 7, 0, 6, 5, 4, 3, 2, 1};
    write_binary(snapshot_one, bytes_one);
    write_binary(snapshot_two, bytes_two);

    Slic3r::ProjectHistoryManager manager(app_data);
    REQUIRE(manager.history_root() == fs::absolute(app_data).lexically_normal() / "project_history" / "v1");

    auto first = manager.commit_snapshot(project, snapshot_one, commit_options("Initial project snapshot", 1000)).get();
    INFO(first.error.message);
    REQUIRE(first.ok());
    REQUIRE(first.committed);
    REQUIRE(first.version.has_value());
    REQUIRE(first.version->snapshot_size == bytes_one.size());
    REQUIRE(first.version->message == "Initial project snapshot");
    REQUIRE(first.repository_path.parent_path() == manager.history_root());
    REQUIRE(first.repository_path.filename().string().size() == 64);
    REQUIRE(first.repository_path != project.parent_path());
    REQUIRE(fs::is_directory(first.repository_path));

    auto duplicate = manager.commit_snapshot(project, snapshot_one, commit_options("Duplicate", 1001)).get();
    INFO(duplicate.error.message);
    REQUIRE(duplicate.ok());
    REQUIRE_FALSE(duplicate.committed);
    REQUIRE(duplicate.version.has_value());
    REQUIRE(duplicate.version->commit_id == first.version->commit_id);
    REQUIRE(duplicate.version->message == "Initial project snapshot");

    auto second = manager.commit_snapshot(project, snapshot_two, commit_options("Edited project", 1002)).get();
    REQUIRE(second.ok());
    REQUIRE(second.committed);
    REQUIRE(second.version.has_value());
    REQUIRE(second.version->snapshot_size == bytes_two.size());

    auto latest_only = manager.list_versions(project, 1).get();
    REQUIRE(latest_only.ok());
    REQUIRE(latest_only.versions.size() == 1);
    REQUIRE(latest_only.versions.front().commit_id == second.version->commit_id);

    auto versions = manager.list_versions(project).get();
    REQUIRE(versions.ok());
    REQUIRE(versions.versions.size() == 2);
    REQUIRE(versions.versions[0].commit_id == second.version->commit_id);
    REQUIRE(versions.versions[1].commit_id == first.version->commit_id);

    const fs::path restored = temporary.path() / "restore" / "old-version.3mf";
    auto           restore  = manager.restore_version(project, first.version->commit_id, restored).get();
    REQUIRE(restore.ok());
    REQUIRE(restore.restored_path == restored);
    REQUIRE(restore.bytes_written == bytes_one.size());
    REQUIRE(read_binary(restored) == bytes_one);

    auto refuses_overwrite = manager.restore_version(project, first.version->commit_id, restored).get();
    REQUIRE_FALSE(refuses_overwrite.ok());
    REQUIRE(refuses_overwrite.error.code == Slic3r::ProjectHistoryErrorCode::DestinationExists);

    const fs::path internal_destination = manager.history_root() / "must-not-write.3mf";
    auto           refuses_internal     = manager.restore_version(project, first.version->commit_id, internal_destination).get();
    REQUIRE_FALSE(refuses_internal.ok());
    REQUIRE(refuses_internal.error.code == Slic3r::ProjectHistoryErrorCode::InvalidArgument);
    REQUIRE_FALSE(fs::exists(internal_destination));
}

TEST_CASE("Project history returns safe validation results without creating repositories", "[project-history]")
{
    TemporaryTree                 temporary;
    Slic3r::ProjectHistoryManager manager(temporary.path() / "app-data");

    const fs::path project = temporary.path() / "model.3mf";
    auto           empty   = manager.list_versions(project).get();
    REQUIRE(empty.ok());
    REQUIRE(empty.versions.empty());
    REQUIRE_FALSE(fs::exists(empty.repository_path));

    const fs::path wrong_extension = temporary.path() / "snapshot.zip";
    write_binary(wrong_extension, {'P', 'K', 3, 4});
    auto invalid_snapshot = manager.commit_snapshot(project, wrong_extension).get();
    REQUIRE_FALSE(invalid_snapshot.ok());
    REQUIRE(invalid_snapshot.error.code == Slic3r::ProjectHistoryErrorCode::InvalidArgument);

    auto invalid_project = manager.list_versions(temporary.path() / "model.stl").get();
    REQUIRE_FALSE(invalid_project.ok());
    REQUIRE(invalid_project.error.code == Slic3r::ProjectHistoryErrorCode::InvalidArgument);

    auto missing_restore = manager.restore_version(project, std::string(40, 'a'), temporary.path() / "missing.3mf").get();
    REQUIRE_FALSE(missing_restore.ok());
    REQUIRE(missing_restore.error.code == Slic3r::ProjectHistoryErrorCode::NotFound);
}

TEST_CASE("Project history identity migration preserves complete ancestry and both identities", "[project-history][migration]")
{
    TemporaryTree                    temporary;
    const fs::path                   app_data      = temporary.path() / "app-data";
    const fs::path                   old_project   = temporary.path() / "projects" / "original.3mf";
    const fs::path                   new_project   = temporary.path() / "projects" / "saved-as.3mf";
    const fs::path                   snapshot_one  = temporary.path() / "snapshots" / "one.3mf";
    const fs::path                   snapshot_two  = temporary.path() / "snapshots" / "two.3mf";
    const fs::path                   snapshot_three = temporary.path() / "snapshots" / "three.3mf";
    const std::vector<unsigned char> bytes_one{'P', 'K', 3, 4, 1};
    const std::vector<unsigned char> bytes_two{'P', 'K', 3, 4, 2};
    const std::vector<unsigned char> bytes_three{'P', 'K', 3, 4, 3};
    write_binary(snapshot_one, bytes_one);
    write_binary(snapshot_two, bytes_two);
    write_binary(snapshot_three, bytes_three);

    Slic3r::ProjectHistoryManager manager(app_data);
    auto first  = manager.commit_snapshot(old_project, snapshot_one, commit_options("Original one", 3000)).get();
    auto second = manager.commit_snapshot(old_project, snapshot_two, commit_options("Original two", 3001)).get();
    REQUIRE(first.ok());
    REQUIRE(second.ok());

    auto migration = manager.migrate_history_identity(old_project, new_project).get();
    INFO(migration.error.message);
    REQUIRE(migration.ok());
    REQUIRE(migration.migrated);
    REQUIRE(migration.source_repository_path == first.repository_path);
    REQUIRE(migration.destination_repository_path != migration.source_repository_path);
    REQUIRE(fs::is_directory(migration.source_repository_path));
    REQUIRE(fs::is_directory(migration.destination_repository_path));

    auto migrated_versions = manager.list_versions(new_project).get();
    REQUIRE(migrated_versions.ok());
    REQUIRE(migrated_versions.versions.size() == 2);
    REQUIRE(migrated_versions.versions[0].commit_id == second.version->commit_id);
    REQUIRE(migrated_versions.versions[1].commit_id == first.version->commit_id);

    auto third = manager.commit_snapshot(new_project, snapshot_three, commit_options("Saved-as edit", 3002)).get();
    REQUIRE(third.ok());
    REQUIRE(third.committed);

    auto new_versions = manager.list_versions(new_project).get();
    REQUIRE(new_versions.ok());
    REQUIRE(new_versions.versions.size() == 3);
    REQUIRE(new_versions.versions[0].commit_id == third.version->commit_id);
    REQUIRE(new_versions.versions[1].commit_id == second.version->commit_id);
    REQUIRE(new_versions.versions[2].commit_id == first.version->commit_id);

    auto old_versions = manager.list_versions(old_project).get();
    REQUIRE(old_versions.ok());
    REQUIRE(old_versions.versions.size() == 2);
    REQUIRE(old_versions.versions[0].commit_id == second.version->commit_id);
    REQUIRE(old_versions.versions[1].commit_id == first.version->commit_id);

    const fs::path restored = temporary.path() / "restored" / "from-migrated-ancestry.3mf";
    auto restore = manager.restore_version(new_project, first.version->commit_id, restored).get();
    REQUIRE(restore.ok());
    REQUIRE(read_binary(restored) == bytes_one);
}

TEST_CASE("Project history identity migration handles missing and equivalent identities safely", "[project-history][migration]")
{
    TemporaryTree                 temporary;
    Slic3r::ProjectHistoryManager manager(temporary.path() / "app-data");
    const fs::path                missing_project = temporary.path() / "missing.3mf";
    const fs::path                new_project     = temporary.path() / "new.3mf";

    auto missing = manager.migrate_history_identity(missing_project, new_project).get();
    INFO(missing.error.message);
    REQUIRE(missing.ok());
    REQUIRE_FALSE(missing.migrated);
    REQUIRE_FALSE(fs::exists(missing.source_repository_path));
    REQUIRE_FALSE(fs::exists(missing.destination_repository_path));

    const fs::path snapshot = temporary.path() / "snapshot.3mf";
    write_binary(snapshot, {'P', 'K', 3, 4, 7});
    auto committed = manager.commit_snapshot(new_project, snapshot, commit_options("Equivalent identity", 3100)).get();
    REQUIRE(committed.ok());

    const fs::path equivalent_project = new_project.parent_path() / "unused-component" / ".." / new_project.filename();
    auto           equivalent         = manager.migrate_history_identity(new_project, equivalent_project).get();
    INFO(equivalent.error.message);
    REQUIRE(equivalent.ok());
    REQUIRE_FALSE(equivalent.migrated);
    REQUIRE(equivalent.source_repository_path == equivalent.destination_repository_path);

    auto invalid_source = manager.migrate_history_identity(temporary.path() / "source.stl", new_project).get();
    REQUIRE_FALSE(invalid_source.ok());
    REQUIRE(invalid_source.error.code == Slic3r::ProjectHistoryErrorCode::InvalidArgument);

    auto invalid_destination = manager.migrate_history_identity(new_project, temporary.path() / "destination.zip").get();
    REQUIRE_FALSE(invalid_destination.ok());
    REQUIRE(invalid_destination.error.code == Slic3r::ProjectHistoryErrorCode::InvalidArgument);
}

TEST_CASE("Project history migration fails closed when destination history exists", "[project-history][migration]")
{
    TemporaryTree                 temporary;
    Slic3r::ProjectHistoryManager manager(temporary.path() / "app-data");
    const fs::path                old_project          = temporary.path() / "old.3mf";
    const fs::path                destination_project  = temporary.path() / "destination.3mf";
    const fs::path                old_snapshot         = temporary.path() / "old-snapshot.3mf";
    const fs::path                destination_snapshot = temporary.path() / "destination-snapshot.3mf";
    const fs::path                save_as_snapshot     = temporary.path() / "save-as-snapshot.3mf";
    write_binary(old_snapshot, {'P', 'K', 3, 4, 1, 1});
    write_binary(destination_snapshot, {'P', 'K', 3, 4, 2, 2});
    write_binary(save_as_snapshot, {'P', 'K', 3, 4, 3, 3});

    auto old_head = manager.commit_snapshot(old_project, old_snapshot, commit_options("Old history", 3200)).get();
    auto destination_head = manager.commit_snapshot(destination_project, destination_snapshot, commit_options("Unrelated destination", 3201)).get();
    REQUIRE(old_head.ok());
    REQUIRE(destination_head.ok());

    auto migration = manager.migrate_history_identity(old_project, destination_project).get();
    REQUIRE_FALSE(migration.ok());
    REQUIRE(migration.error.code == Slic3r::ProjectHistoryErrorCode::DestinationExists);
    REQUIRE_FALSE(migration.migrated);

    auto composite = manager
                         .migrate_then_commit_snapshot(old_project, destination_project, save_as_snapshot, commit_options("Must not append", 3202))
                         .get();
    REQUIRE_FALSE(composite.ok());
    REQUIRE(composite.error.code == Slic3r::ProjectHistoryErrorCode::DestinationExists);
    REQUIRE_FALSE(composite.committed);
    REQUIRE_FALSE(composite.history_migrated);

    auto destination_versions = manager.list_versions(destination_project).get();
    REQUIRE(destination_versions.ok());
    REQUIRE(destination_versions.versions.size() == 1);
    REQUIRE(destination_versions.versions.front().commit_id == destination_head.version->commit_id);
    REQUIRE(destination_versions.versions.front().message == "Unrelated destination");

    auto old_versions = manager.list_versions(old_project).get();
    REQUIRE(old_versions.ok());
    REQUIRE(old_versions.versions.size() == 1);
    REQUIRE(old_versions.versions.front().commit_id == old_head.version->commit_id);
}

TEST_CASE("Project history drains a queued migration and commit during shutdown", "[project-history][migration]")
{
    TemporaryTree  temporary;
    const fs::path old_project  = temporary.path() / "queued-old.3mf";
    const fs::path new_project  = temporary.path() / "queued-new.3mf";
    const fs::path first_path   = temporary.path() / "queued-first.3mf";
    const fs::path second_path  = temporary.path() / "queued-second.3mf";
    write_binary(first_path, {'P', 'K', 3, 4, 1});
    write_binary(second_path, {'P', 'K', 3, 4, 2});

    auto manager = std::make_unique<Slic3r::ProjectHistoryManager>(temporary.path() / "app-data");
    auto first_future = manager->commit_snapshot(old_project, first_path, commit_options("Before Save As", 3300));
    auto save_as_future = manager->migrate_then_commit_snapshot(old_project, new_project, second_path, commit_options("Saved As", 3301));
    manager.reset();

    auto first   = first_future.get();
    auto save_as = save_as_future.get();
    INFO(first.error.message);
    REQUIRE(first.ok());
    INFO(save_as.error.message);
    REQUIRE(save_as.ok());
    REQUIRE(save_as.history_migrated);
    REQUIRE(save_as.committed);
    REQUIRE(save_as.previous_repository_path == first.repository_path);

    Slic3r::ProjectHistoryManager reopened(temporary.path() / "app-data");
    auto                          versions = reopened.list_versions(new_project).get();
    REQUIRE(versions.ok());
    REQUIRE(versions.versions.size() == 2);
    REQUIRE(versions.versions[0].message == "Saved As");
    REQUIRE(versions.versions[1].commit_id == first.version->commit_id);
}

TEST_CASE("Project history serializes queued commits and drains them during shutdown", "[project-history]")
{
    TemporaryTree  temporary;
    const fs::path project     = temporary.path() / "queued-project.3mf";
    const fs::path first_path  = temporary.path() / "first.3mf";
    const fs::path second_path = temporary.path() / "second.3mf";
    write_binary(first_path, {'P', 'K', 3, 4, 1});
    write_binary(second_path, {'P', 'K', 3, 4, 2});

    auto manager       = std::make_unique<Slic3r::ProjectHistoryManager>(temporary.path() / "app-data");
    auto first_future  = manager->commit_snapshot(project, first_path, commit_options("Queued first", 2000));
    auto second_future = manager->commit_snapshot(project, second_path, commit_options("Queued second", 2001));
    manager.reset();

    auto first  = first_future.get();
    auto second = second_future.get();
    INFO(first.error.message);
    REQUIRE(first.ok());
    REQUIRE(first.committed);
    INFO(second.error.message);
    REQUIRE(second.ok());
    REQUIRE(second.committed);

    Slic3r::ProjectHistoryManager reopened(temporary.path() / "app-data");
    auto                          versions = reopened.list_versions(project).get();
    REQUIRE(versions.ok());
    REQUIRE(versions.versions.size() == 2);
    REQUIRE(versions.versions[0].message == "Queued second");
    REQUIRE(versions.versions[1].message == "Queued first");
}

TEST_CASE("Project history serializes independent managers for one project identity", "[project-history][concurrency]")
{
    TemporaryTree  temporary;
    const fs::path app_data = temporary.path() / "app-data";
    const fs::path project  = temporary.path() / "shared-project.3mf";

    Slic3r::ProjectHistoryManager first_manager(app_data);
    Slic3r::ProjectHistoryManager second_manager(app_data);

    constexpr std::size_t commit_count = 12;
    std::vector<std::future<Slic3r::ProjectHistoryCommitResult>> futures;
    std::set<std::string>                                        expected_messages;
    futures.reserve(commit_count);
    for (std::size_t index = 0; index < commit_count; ++index) {
        const fs::path snapshot = temporary.path() / ("concurrent-" + std::to_string(index) + ".3mf");
        std::vector<unsigned char> bytes(128u * 1024u, static_cast<unsigned char>(index + 1));
        bytes[0] = 'P';
        bytes[1] = 'K';
        bytes[2] = 3;
        bytes[3] = 4;
        write_binary(snapshot, bytes);

        const std::string message = "Independent manager " + std::to_string(index);
        expected_messages.emplace(message);
        Slic3r::ProjectHistoryManager &manager = index % 2 == 0 ? first_manager : second_manager;
        futures.emplace_back(manager.commit_snapshot(project, snapshot, commit_options(message, 4000 + static_cast<std::int64_t>(index))));
    }

    fs::path              repository_path;
    std::set<std::string> commit_ids;
    for (auto &future : futures) {
        auto result = future.get();
        INFO(result.error.message);
        REQUIRE(result.ok());
        REQUIRE(result.committed);
        REQUIRE(result.version.has_value());
        repository_path = result.repository_path;
        commit_ids.emplace(result.version->commit_id);
    }
    REQUIRE(commit_ids.size() == commit_count);

    auto versions = first_manager.list_versions(project).get();
    INFO(versions.error.message);
    REQUIRE(versions.ok());
    REQUIRE(versions.versions.size() == commit_count);
    std::set<std::string> actual_messages;
    for (const auto &version : versions.versions) actual_messages.emplace(version.message);
    REQUIRE(actual_messages == expected_messages);

    const fs::path lock_path = app_data / "project_history" / "locks" / "v1" / (repository_path.filename().string() + ".lock");
    REQUIRE(fs::is_regular_file(lock_path));
    REQUIRE(lock_path.parent_path() != repository_path);
    for (const auto &entry : fs::directory_iterator(first_manager.history_root()))
        REQUIRE(entry.path().filename().string().rfind(".create-", 0) != 0);

#ifndef _WIN32
    constexpr fs::perms non_owner_permissions = fs::perms::group_all | fs::perms::others_all;
    REQUIRE((fs::status(repository_path).permissions() & non_owner_permissions) == fs::perms::none);
    REQUIRE((fs::status(lock_path).permissions() & non_owner_permissions) == fs::perms::none);
#endif
}

} // namespace
