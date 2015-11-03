#include <string.h>
#include <strings.h>

#include "unity.h"
#include "unity_fixture.h"

#include "constants.h"
#include "blocks_handler.h"
#include "disk_emulator.h"
#include "memory_structures.h"


TEST_GROUP_RUNNER(TestBlocksHandler) {
	RUN_TEST_CASE(TestBlocksHandler, get_block_number_of_first_datablock);
	RUN_TEST_CASE(TestBlocksHandler, get_ith_block_number_in_datablock__returns_ID_of_datablock_placed_in_ith_position_in_datablock);
	RUN_TEST_CASE(TestBlocksHandler, set_ith_block_number_in_datablock__sets_datablock_ID_at_ith_position_in_datablock);
	RUN_TEST_CASE(TestBlocksHandler, has_at_least_one_datablock_number_left_without_pointer__returns_0_if_there_is_only_pointer_to_next_datablock_or_no_pointer_at_all);
	RUN_TEST_CASE(TestBlocksHandler, shift_datablock_numbers_in_buffer_to_left_except_pointer_to_next_block);
	RUN_TEST_CASE(TestBlocksHandler, data_block_alloc__allocates_correctly_a_datablock);
	RUN_TEST_CASE(TestBlocksHandler, is_datablock_full_of_free_datablock_numbers);
	RUN_TEST_CASE(TestBlocksHandler, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock);
	RUN_TEST_CASE(TestBlocksHandler, data_block_free);
	RUN_TEST_CASE(TestBlocksHandler, bread);
	RUN_TEST_CASE(TestBlocksHandler, bwrite);
}




TEST_GROUP(TestBlocksHandler);


// To be executed before each test case
TEST_SETUP(TestBlocksHandler) {
}

// To be executed after each test
TEST_TEAR_DOWN(TestBlocksHandler) {
}


TEST(TestBlocksHandler, get_block_number_of_first_datablock) {
	int block_size, number_of_inodes, inode_size;

	block_size = 4096;
	number_of_inodes = 26;
	inode_size = 256;

	struct superblock sp;
	sp.block_size = (short) block_size;
	sp.number_of_inodes = (short) number_of_inodes;
	sp.inode_size = (short)inode_size;

	TEST_ASSERT_EQUAL(3, get_block_number_of_first_datablock(&sp));

	sp.number_of_inodes = 1031;
	TEST_ASSERT_EQUAL(66, get_block_number_of_first_datablock(&sp));
}

TEST(TestBlocksHandler, get_ith_block_number_in_datablock__returns_ID_of_datablock_placed_in_ith_position_in_datablock) {
	char datablock[DATA_BLOCK_SIZE];

	bzero(datablock, DATA_BLOCK_SIZE); // Write zeros everywhere in block as it should be when block is empty

	int block_number_1_to_be_copied = 1039;
	int block_number_2_to_be_copied = 23567;
	int block_number_3_to_be_copied = 2;

	memcpy(&datablock[0], &block_number_1_to_be_copied, sizeof(int));
	memcpy(&datablock[4], &block_number_2_to_be_copied, sizeof(int));
	memcpy(&datablock[8], &block_number_3_to_be_copied, sizeof(int));

	TEST_ASSERT_EQUAL(block_number_1_to_be_copied, get_ith_block_number_in_datablock(datablock, 1));
	TEST_ASSERT_EQUAL(block_number_2_to_be_copied, get_ith_block_number_in_datablock(datablock, 2));
	TEST_ASSERT_EQUAL(block_number_3_to_be_copied, get_ith_block_number_in_datablock(datablock, 3));
}

TEST(TestBlocksHandler, set_ith_block_number_in_datablock__sets_datablock_ID_at_ith_position_in_datablock) {
	char datablock[DATA_BLOCK_SIZE];

	bzero(datablock, DATA_BLOCK_SIZE); // Write zeros everywhere in block as it should be when block is empty

	int block_number_1_to_be_copied = 1039;
	int block_number_2_to_be_copied = 23567;
	int block_number_3_to_be_copied = 2;

	set_ith_block_number_in_datablock(datablock, 1, block_number_1_to_be_copied);
	set_ith_block_number_in_datablock(datablock, 2, block_number_2_to_be_copied);
	set_ith_block_number_in_datablock(datablock, 3, block_number_3_to_be_copied);

	TEST_ASSERT_EQUAL(block_number_1_to_be_copied, get_ith_block_number_in_datablock(datablock, 1));
	TEST_ASSERT_EQUAL(block_number_2_to_be_copied, get_ith_block_number_in_datablock(datablock, 2));
	TEST_ASSERT_EQUAL(block_number_3_to_be_copied, get_ith_block_number_in_datablock(datablock, 3));
}

TEST(TestBlocksHandler, has_at_least_one_datablock_number_left_without_pointer__returns_0_if_there_is_only_pointer_to_next_datablock_or_no_pointer_at_all) {
	char datablock[DATA_BLOCK_SIZE];

	bzero(datablock, DATA_BLOCK_SIZE); // Write zeros everywhere in block as it should be when block is empty

	int pointer_to_next_datablock_containing_IDs_of_free_datablocks = 1039;
	int block_number_1 = 23567;

	set_ith_block_number_in_datablock(datablock, 1, pointer_to_next_datablock_containing_IDs_of_free_datablocks);

	TEST_ASSERT_EQUAL(0, has_at_least_one_datablock_number_left_without_pointer(datablock));

	set_ith_block_number_in_datablock(datablock, 2, block_number_1);
	TEST_ASSERT_EQUAL(1, has_at_least_one_datablock_number_left_without_pointer(datablock));
}

TEST(TestBlocksHandler, shift_datablock_numbers_in_buffer_to_left_except_pointer_to_next_block) {
	char datablock[DATA_BLOCK_SIZE];
	int number_of_block_numbers_in_datablock;

	bzero(datablock, DATA_BLOCK_SIZE); // Write zeros everywhere in block as it should be when block is empty
	number_of_block_numbers_in_datablock = DATA_BLOCK_SIZE / sizeof(int);

	// We set the position of the block number as the block number in the buffer for the purpose of the test
	for (int block_number = 1; block_number <= number_of_block_numbers_in_datablock; block_number++) {
		set_ith_block_number_in_datablock(datablock, block_number, block_number);
	}

	shift_datablock_numbers_in_buffer_to_left_except_pointer_to_next_block(datablock);

	// Assert 1st block didn't change
	TEST_ASSERT_EQUAL(1, get_ith_block_number_in_datablock(datablock, 1));

	// Assert all the other blocks left shifted (block number 2 disappeared)
	for (int i = 3; i <= number_of_block_numbers_in_datablock; i++) {
		TEST_ASSERT_EQUAL(i, get_ith_block_number_in_datablock(datablock, i-1));
	}

	// Make sure that last spot for a block number is empty (only 0s)
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(datablock, number_of_block_numbers_in_datablock));
}

TEST(TestBlocksHandler, data_block_alloc__allocates_correctly_a_datablock) {
	struct superblock * sb;
	int first_datablock_position, first_free_datablock_number, second_free_datablock_number, pointer_next_free_datablock_list;
	int first_free_datablock_number_in_second_list;
	char datablock_buffer[DATA_BLOCK_SIZE];
	char datablock_buffer2[DATA_BLOCK_SIZE];
	char free_block_buffer[DATA_BLOCK_SIZE];

	struct data_block * result_datablock;

	// BEGINNING OF SETUP for the test
	init_disk_emulator();

	sb = get_superblock();
	sb->block_size = 4096;
	sb->number_of_inodes = 150;
	sb->inode_size = 256;
	sb->free_data_blocks = 560;

	first_datablock_position = get_block_number_of_first_datablock(sb);

	pointer_next_free_datablock_list = first_datablock_position + 7; // These are random but they have to be after 1st datablock
	first_free_datablock_number = first_datablock_position + 4; 
	second_free_datablock_number = first_datablock_position + 2;

	memset(datablock_buffer, 0, DATA_BLOCK_SIZE); // Set 0s to the datablock before setting datablock numbers

	// We set here the IDs of the free datablocks in the superblock free datablock list
	// Set pointer to next block
	set_ith_block_number_in_datablock(datablock_buffer, 1, pointer_next_free_datablock_list);

	// Set 1st free datablock number
	set_ith_block_number_in_datablock(datablock_buffer, 2, first_free_datablock_number);
	set_ith_block_number_in_datablock(datablock_buffer, 3, second_free_datablock_number);

	// We write to disk the datablock containing the list of free datablocks
	write_block(first_datablock_position, datablock_buffer, DATA_BLOCK_SIZE);

	// We write the IDs of the free datablocks in the datablock that is pointed by the 1st (superblock free datablock list) datablock
	memset(datablock_buffer2, 0, DATA_BLOCK_SIZE); // Set 0s to the datablock before setting datablock numbers
	first_free_datablock_number_in_second_list = first_datablock_position + 11;
	set_ith_block_number_in_datablock(datablock_buffer2, 1, 0); // We say that there's no pointer to a 3rd datablock.
	set_ith_block_number_in_datablock(datablock_buffer2, 2, first_free_datablock_number_in_second_list);

	// We write to disk the datablock containing the 2nd list of free datablocks
	write_block(pointer_next_free_datablock_list, datablock_buffer2, DATA_BLOCK_SIZE);

	// We write a bunch of random stuff in the blocks that are supposed to be free to see if the data will be cleared
	strcpy(free_block_buffer, "Hey bro! How is it going?");
	write_block(first_free_datablock_number, free_block_buffer, DATA_BLOCK_SIZE);
	write_block(second_free_datablock_number, free_block_buffer, DATA_BLOCK_SIZE);
	write_block(first_free_datablock_number_in_second_list, free_block_buffer, DATA_BLOCK_SIZE);

	// END OF SETUP

	TEST_ASSERT_EQUAL(560, sb->free_data_blocks);

	// We assert that we get the right free datablock numbers during the allocation
	result_datablock = data_block_alloc();
	TEST_ASSERT_EQUAL(first_free_datablock_number, result_datablock->data_block_id);
	read_block(first_free_datablock_number, free_block_buffer); // We want to check whether the block got cleared on disk
	TEST_ASSERT_EQUAL(0, free_block_buffer[0]); // Assert that block got cleared
	TEST_ASSERT_EQUAL(0, free_block_buffer[1]);
	TEST_ASSERT_EQUAL(559, sb->free_data_blocks);
	// free(result_datablock);

	result_datablock = data_block_alloc();
	TEST_ASSERT_EQUAL(second_free_datablock_number, result_datablock->data_block_id);
	read_block(second_free_datablock_number, free_block_buffer); // We want to check whether the block got cleared on disk
	TEST_ASSERT_EQUAL(0, free_block_buffer[0]); // Assert that block got cleared
	TEST_ASSERT_EQUAL(0, free_block_buffer[1]);
	TEST_ASSERT_EQUAL(558, sb->free_data_blocks);
	// free(result_datablock);

	// Now that should be the block that was containing the 2nd part of the list of free datablock IDS that should be given back
	result_datablock = data_block_alloc();
	TEST_ASSERT_EQUAL_MESSAGE(pointer_next_free_datablock_list, result_datablock->data_block_id, "WASFD");
	read_block(pointer_next_free_datablock_list, free_block_buffer); // We want to check whether the block got cleared on disk
	TEST_ASSERT_EQUAL(0, free_block_buffer[0]); // Assert that block got cleared
	TEST_ASSERT_EQUAL(0, free_block_buffer[1]);
	TEST_ASSERT_EQUAL(557, sb->free_data_blocks);
	// free(result_datablock);

	result_datablock = data_block_alloc();
	TEST_ASSERT_EQUAL(first_free_datablock_number_in_second_list, result_datablock->data_block_id);
	read_block(first_free_datablock_number_in_second_list, free_block_buffer); // We want to check whether the block got cleared on disk
	TEST_ASSERT_EQUAL(0, free_block_buffer[0]); // Assert that block got cleared
	TEST_ASSERT_EQUAL(0, free_block_buffer[1]);
	TEST_ASSERT_EQUAL(556, sb->free_data_blocks);
	// free(result_datablock);

	// At this point we have no more free block numbers stored so we should get NULL
	result_datablock = data_block_alloc();
	TEST_ASSERT_NULL(result_datablock);
	TEST_ASSERT_EQUAL(556, sb->free_data_blocks);

	free_disk_emulator();
}

TEST(TestBlocksHandler, is_datablock_full_of_free_datablock_numbers) {
	char full_datablock[DATA_BLOCK_SIZE], almost_full_datablock[DATA_BLOCK_SIZE], empty_datablock[DATA_BLOCK_SIZE];
	int i;

	// We set all the datablocks with 0s initially
	memset(full_datablock, 0, DATA_BLOCK_SIZE);
	memset(almost_full_datablock, 0, DATA_BLOCK_SIZE);
	memset(empty_datablock, 0, DATA_BLOCK_SIZE);
	
	// We make one datablock completely full
	for (i = 1; (i * sizeof(int)) <= DATA_BLOCK_SIZE; i++) {
		set_ith_block_number_in_datablock(full_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	// We make one datablock almost completely full (last spot is empty)
	for (i = 1; (i * sizeof(int)) <= (DATA_BLOCK_SIZE - sizeof(int)); i++) {
		set_ith_block_number_in_datablock(almost_full_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	TEST_ASSERT_EQUAL(1, is_datablock_full_of_free_datablock_numbers(full_datablock));
	TEST_ASSERT_EQUAL(0, is_datablock_full_of_free_datablock_numbers(almost_full_datablock));
	TEST_ASSERT_EQUAL(0, is_datablock_full_of_free_datablock_numbers(empty_datablock));
}

TEST(TestBlocksHandler, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock) {
	char full_datablock[DATA_BLOCK_SIZE], almost_full_datablock[DATA_BLOCK_SIZE], 
		free_spots_datablock[DATA_BLOCK_SIZE], empty_datablock[DATA_BLOCK_SIZE];
	int i, last_ith_filled, last_block_rank_position;

	// We set all the datablocks with 0s initially
	memset(full_datablock, 0, DATA_BLOCK_SIZE);
	memset(almost_full_datablock, 0, DATA_BLOCK_SIZE);
	memset(free_spots_datablock, 0, DATA_BLOCK_SIZE);
	memset(empty_datablock, 0, DATA_BLOCK_SIZE);

	// We make one datablock completely full
	for (i = 1; (i * sizeof(int)) <= DATA_BLOCK_SIZE; i++) {
		set_ith_block_number_in_datablock(full_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	// We make one datablock almost completely full (last spot is empty)
	last_block_rank_position = 1024; // 4096/4 = DATA_BLOCK_SIZE / sizeof(int)
	for (i = 1; (i * sizeof(int)) <= (DATA_BLOCK_SIZE - sizeof(int)); i++) {
		set_ith_block_number_in_datablock(almost_full_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	// We make one datablock half filled up to a certain position
	last_ith_filled = 120;

	for (i = 1; (i * sizeof(int)) <= (last_ith_filled * sizeof(int)); i++) {
		set_ith_block_number_in_datablock(free_spots_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	TEST_ASSERT_EQUAL(-1, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(full_datablock));
	TEST_ASSERT_EQUAL(last_block_rank_position, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(almost_full_datablock));
	TEST_ASSERT_EQUAL((last_ith_filled + 1), get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(free_spots_datablock));
	TEST_ASSERT_EQUAL(1, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(empty_datablock));
}

TEST(TestBlocksHandler, data_block_free) {
	struct superblock * sb;
	int first_datablock_position, pointer_to_second_datablock, pointer_to_third_datablock;
	char first_datablock[DATA_BLOCK_SIZE], second_datablock[DATA_BLOCK_SIZE];
	char read_buffer[DATA_BLOCK_SIZE];

	// BEGINNING OF SETUP for the test
	init_disk_emulator();

	sb = get_superblock();
	sb->block_size = 4096;
	sb->number_of_inodes = 150;
	sb->inode_size = 256;
	sb->free_data_blocks = 1024;

	first_datablock_position = get_block_number_of_first_datablock(sb);

	pointer_to_second_datablock = 530; // These are random but they have to be after 1st datablock

	// We make the 1st datablock almost full in terms of datablock number (there's room just for a last datablock number)
	memset(first_datablock, 0, DATA_BLOCK_SIZE); // Set 0s to the datablock before setting datablock numbers
	for (int i = 1; (i * sizeof(int)) <= (DATA_BLOCK_SIZE - sizeof(int)); i++) {
		set_ith_block_number_in_datablock(first_datablock, i, i);
	}
	set_ith_block_number_in_datablock(first_datablock, 1, 530); // Set pointer to second datablock

	// We write to disk the datablock containing the list of free datablocks
	write_block(first_datablock_position, first_datablock, DATA_BLOCK_SIZE);

	pointer_to_third_datablock = 550;
	memset(second_datablock, 0, DATA_BLOCK_SIZE); // Set 0s to the datablock before setting datablock numbers
	set_ith_block_number_in_datablock(second_datablock, 1, pointer_to_third_datablock); // Set pointer to third datablock (we won't create it)
	set_ith_block_number_in_datablock(second_datablock, 2, 506); // Set a random free datablock number in second datablock list

	// We write to disk the datablock containing the list of free datablocks
	write_block(pointer_to_second_datablock, second_datablock, DATA_BLOCK_SIZE);

	// END OF SETUP


	// We free a datablock
	struct data_block datablock_to_free_1;
	datablock_to_free_1.data_block_id = 495; 
	memset(datablock_to_free_1.block, 1, DATA_BLOCK_SIZE); // Put random content in datablock to be freed

	TEST_ASSERT_EQUAL(0, data_block_free(&datablock_to_free_1));

	// Make sure 1st datablock contain right datablock numbers
	read_block(first_datablock_position, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_second_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(2, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(3, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(495, get_ith_block_number_in_datablock(read_buffer, DATA_BLOCK_SIZE/sizeof(int))); // 4096/4 = DATA_BLOCK_SIZE/sizeof(int) = last blocknumber

	// Make sure 2nd datablock contain the 2 right datablock numbers (pointer to 3rd datablock and another datablock number)
	read_block(pointer_to_second_datablock, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_third_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(506, get_ith_block_number_in_datablock(read_buffer, 2));

	// We make sure the datablock that was freed has only 0s in its content
	read_block(495, read_buffer);
	for (int i = 0; i < DATA_BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL(0, (int)read_buffer[i]);
	}



	// We free another datablock
	struct data_block datablock_to_free_2;
	datablock_to_free_2.data_block_id = 497; 
	memset(datablock_to_free_1.block, 1, DATA_BLOCK_SIZE); // Put random content in datablock to be freed

	// We free a datablock
	TEST_ASSERT_EQUAL(0, data_block_free(&datablock_to_free_2));

	// Make sure 1st datablock contain only a pointer to the newly freed datablock and the pointed datablock contain what was in the 1st datablock
	read_block(first_datablock_position, read_buffer);
	TEST_ASSERT_EQUAL(497, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, DATA_BLOCK_SIZE/sizeof(int)));

	// Make sure the newly freed datablock contains datablock list that was contained in the 1st datablock before we freed the new datablock
	read_block(497, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_second_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(2, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(3, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(495, get_ith_block_number_in_datablock(read_buffer, DATA_BLOCK_SIZE/sizeof(int)));

	// Make sure 3rd datablock contain teh remaining datablock numbers
	read_block(pointer_to_second_datablock, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_third_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(506, get_ith_block_number_in_datablock(read_buffer, 2));



	// We free another datablock
	struct data_block datablock_to_free_3;
	datablock_to_free_3.data_block_id = 499; 
	memset(datablock_to_free_3.block, 1, DATA_BLOCK_SIZE); // Put random content in datablock to be freed

	// We free a datablock
	TEST_ASSERT_EQUAL(0, data_block_free(&datablock_to_free_3));

	// Make sure 1st datablock contain only a pointer to next datablock and the newly added free datablock
	read_block(first_datablock_position, read_buffer);
	TEST_ASSERT_EQUAL(497, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(499, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, DATA_BLOCK_SIZE/sizeof(int)));

	// Make sure the newly freed datablock contains datablock list that was contained in the 1st datablock before we freed the new datablock
	read_block(497, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_second_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(2, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(3, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(495, get_ith_block_number_in_datablock(read_buffer, DATA_BLOCK_SIZE/sizeof(int)));

	// Make sure 3rd datablock contain teh remaining datablock numbers
	read_block(pointer_to_second_datablock, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_third_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(506, get_ith_block_number_in_datablock(read_buffer, 2));

	free_disk_emulator();
}

TEST(TestBlocksHandler, bread) {
	struct data_block * datablock;
	char read_buffer[DATA_BLOCK_SIZE], buffer[DATA_BLOCK_SIZE];

	init_disk_emulator();

	read_block(234, read_buffer);
	for (int i = 0; i < DATA_BLOCK_SIZE; i++) {
		buffer[i] = 'd';
		TEST_ASSERT_FALSE(read_buffer[i] == 'd');
	}

	write_block(234, buffer, DATA_BLOCK_SIZE);

	
	datablock = bread(234);
	TEST_ASSERT_EQUAL(234, datablock->data_block_id);
	
	for (int i = 0; i < DATA_BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL('d', datablock->block[i]);
	}
}

TEST(TestBlocksHandler, bwrite) {
	struct data_block datablock;
	char read_buffer[DATA_BLOCK_SIZE];

	init_disk_emulator();

	datablock.data_block_id = 121;

	for (int i = 0; i < DATA_BLOCK_SIZE; i++) {
		datablock.block[i] = 'e';
	}

	TEST_ASSERT_EQUAL(0, bwrite(&datablock));

	read_block(121, read_buffer);
	
	for (int i = 0; i < DATA_BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL('e', read_buffer[i]);
	}
}
