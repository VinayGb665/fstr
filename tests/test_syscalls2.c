#include "unity.h"
#include "unity_fixture.h"

#include "syscalls2.h"


TEST_GROUP_RUNNER(TestSyscalls2) {
	RUN_TEST_CASE(TestSyscalls2, allocate_file_descriptor_table);
	RUN_TEST_CASE(TestSyscalls2, get_file_descriptor_table);
	RUN_TEST_CASE(TestSyscalls2, delete_file_descriptor_table);
	RUN_TEST_CASE(TestSyscalls2, find_available_fd);
	RUN_TEST_CASE(TestSyscalls2, allocate_file_descriptor_entry);
	RUN_TEST_CASE(TestSyscalls2, allocate_file_descriptor_entry__when_table_is_full);
}




TEST_GROUP(TestSyscalls2);


// To be executed before each test case
TEST_SETUP(TestSyscalls2) {
}

// To be executed after each test
TEST_TEAR_DOWN(TestSyscalls2) {
}



TEST(TestSyscalls2, allocate_file_descriptor_table) {
	struct file_descriptor_table *table;
	table = allocate_file_descriptor_table(2112);

	TEST_ASSERT_EQUAL(2112, table->pid);
	TEST_ASSERT_EQUAL(8, table->total_descriptors);

	int i;

	// Make sure the 1st file descriptors are set properly
	TEST_ASSERT_EQUAL(0, table->entries[0].fd);
	TEST_ASSERT_EQUAL(1, table->entries[1].fd);
	TEST_ASSERT_EQUAL(2, table->entries[2].fd);

	for (i = 3; i < 8; i++) {
		TEST_ASSERT_EQUAL(FD_NOT_USED, table->entries[i].fd); // Make sure file descriptor is set to unused
	}

	table = allocate_file_descriptor_table(2112);
	TEST_ASSERT_NULL(table);

	delete_file_descriptor_table(2112);
}

TEST(TestSyscalls2, get_file_descriptor_table) {
	struct file_descriptor_table *table, *result_table;

	TEST_ASSERT_NULL(get_file_descriptor_table(2112));
	table = allocate_file_descriptor_table(2112);

	table->total_descriptors = 3;

	result_table = get_file_descriptor_table(2112);
	TEST_ASSERT_EQUAL(2112, result_table->pid);
	TEST_ASSERT_EQUAL(3, result_table->total_descriptors);

	delete_file_descriptor_table(2112);
}

TEST(TestSyscalls2, delete_file_descriptor_table) {
	struct file_descriptor_table *table, *result_table;

	table = allocate_file_descriptor_table(2112);
	TEST_ASSERT_FALSE(table == NULL);

	delete_file_descriptor_table(2112);
	result_table = get_file_descriptor_table(2112);
	TEST_ASSERT_NULL(result_table);
}

TEST(TestSyscalls2, find_available_fd) {
	struct file_descriptor_table *table;

	table = allocate_file_descriptor_table(2112);

	TEST_ASSERT_EQUAL(-1, find_available_fd(4));
	TEST_ASSERT_EQUAL(3, find_available_fd(2112));

	table->entries[3].fd = 3;
	TEST_ASSERT_EQUAL(4, find_available_fd(2112));

	table->used_descriptors = table->total_descriptors;
	TEST_ASSERT_EQUAL(-1, find_available_fd(2112));

	delete_file_descriptor_table(2112);
}

TEST(TestSyscalls2, allocate_file_descriptor_entry) {
	struct file_descriptor_table *table;
	struct file_descriptor_entry *entry;
	
	entry = allocate_file_descriptor_entry(1632);

	table = get_file_descriptor_table(1632);
	TEST_ASSERT_FALSE(table == NULL);
	TEST_ASSERT_EQUAL(1632, table->pid);
	TEST_ASSERT_EQUAL(&table->entries[3], entry);

	TEST_ASSERT_EQUAL(4, table->used_descriptors);
	TEST_ASSERT_EQUAL(3, entry->fd);

	delete_file_descriptor_table(1632);
}

TEST(TestSyscalls2, allocate_file_descriptor_entry__when_table_is_full) {
	struct file_descriptor_table *table;
	struct file_descriptor_entry *entry, *entry2;
	
	entry = allocate_file_descriptor_entry(1632);

	table = get_file_descriptor_table(1632);
	TEST_ASSERT_EQUAL(8, table->total_descriptors);
	TEST_ASSERT_EQUAL(3, entry->fd);

	allocate_file_descriptor_entry(1632);
	allocate_file_descriptor_entry(1632);
	allocate_file_descriptor_entry(1632);
	allocate_file_descriptor_entry(1632);

	entry2 = allocate_file_descriptor_entry(1632);
	TEST_ASSERT_EQUAL(8, entry2->fd);
	TEST_ASSERT_EQUAL(16, table->total_descriptors);

	// Make sure previous file descriptor entries are intact
	TEST_ASSERT_EQUAL(3, entry->fd);

	delete_file_descriptor_table(1632);
}
