/******************************************************************************/
/**
@file		test_open_adress_hash_dictionary_handler.c
@author		Scott Ronald Fazackerley
@brief		Tests base operations for open address hash dict
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#include "test_open_address_hash_dictionary_handler.h"

#define ION_MAX_HASH_TEST 100

/**
@brief		A helper function to build a test dictionary instance

@param	  map_handler
@param	  record
@param	  size
@param	  test_dictionary
*/
void
createTestDictionary(
	ion_dictionary_handler_t	*map_handler,
	const ion_record_info_t		*record,
	int							size,
	ion_dictionary_t			*test_dictionary,
	ion_key_type_t				key_type
) {
	oadict_init(map_handler);	/* register handler for hashmap */
	/* register the appropriate handler for a given dictionary */

	dictionary_create(map_handler, test_dictionary, 1, key_type, record->key_size, record->value_size, size);

	/* build test relation */
	int			i;
	ion_value_t str;

	str = malloc(record->value_size);

	for (i = 0; i < size; i++) {
		sprintf((char *) str, "value : %i", i);
		test_dictionary->handler->insert(test_dictionary, &i, str);
	}

	free(str);
}

/**
@brief		Validates the correct registration of function pointers
			for open address dictionary structure.

@param	  tc
				Test case.
*/
void
test_open_address_hashmap_handler_function_registration(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t map_handler;	/* create handler for hashmap */

	oadict_init(&map_handler);	/* register handler for hashmap */

	/* check to ensure that function pointers are correctly registered */
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.insert == &oadict_insert);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.create_dictionary == &oadict_create_dictionary);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.update == &oadict_update);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.remove == &oadict_delete);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.delete_dictionary == &oadict_delete_dictionary);
}

/**
@brief	  Tests for creatation and deletion of open address hash.

@param	  tc
				Test case.
*/
void
test_open_address_hashmap_handler_create_destroy(
	planck_unit_test_t *tc
) {
	int					size;
	ion_record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	ion_dictionary_handler_t map_handler;	/* create handler for hashmap */

	oadict_init(&map_handler);	/* register handler for hashmap */

	/* dictionary handler for test instance */
	ion_dictionary_t test_dictionary;

	/* register the appropriate handler for a given dictionary */
	dictionary_create(&map_handler, &test_dictionary, 1, key_type_numeric_signed, record.key_size, record.value_size, size);

	PLANCK_UNIT_ASSERT_TRUE(tc, (((ion_hashmap_t *) test_dictionary.instance)->super.record.key_size) == record.key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((ion_hashmap_t *) test_dictionary.instance)->super.record.value_size) == record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((ion_hashmap_t *) test_dictionary.instance)->map_size) == size);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((ion_hashmap_t *) test_dictionary.instance)->compute_hash) == &oah_compute_simple_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((ion_hashmap_t *) test_dictionary.instance)->write_concern) == wc_insert_unique);
	PLANCK_UNIT_ASSERT_TRUE(tc, test_dictionary.handler->delete_dictionary(&test_dictionary) == err_ok);
	PLANCK_UNIT_ASSERT_TRUE(tc, test_dictionary.instance == NULL);
}

void
test_open_address_dictionary_cursor_equality(
	planck_unit_test_t *tc
) {
	int					size;
	ion_record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	ion_dictionary_handler_t	map_handler;			/* create handler for hashmap */
	ion_dictionary_t			test_dictionary;		/* dictionary handler for test instance */

	createTestDictionary(&map_handler, &record, size, &test_dictionary, key_type_numeric_signed);

	ion_dict_cursor_t *cursor;	/* create a new cursor pointer */

	/* create a new predicate statement */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1, int));

	/* test that the query runs on dictionary instance okay */
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dictionary_find(&test_dictionary, &predicate, &cursor));

	/* check the status of the cursor as it should be initialized */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	/* destroy the cursor */
	cursor->destroy(&cursor);

	/* and check that cursor has been destroyed correctly */
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	/* and destroy the dictionary instance */
	dictionary_delete_dictionary(&test_dictionary);
}

void
test_open_address_dictionary_handler_query_with_results(
	planck_unit_test_t *tc
) {
	int					size;
	ion_record_info_t	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size	= sizeof(int);
	record_info.value_size	= 10;
	size					= 10;

	ion_dictionary_handler_t	map_handler;			/* create handler for hashmap */
	ion_dictionary_t			test_dictionary;		/* dictionary handler for test instance */

	createTestDictionary(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	ion_dict_cursor_t *cursor;	/* create a new cursor pointer */

	/* create a new predicate statement */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1, int));

	/* test that the query runs on dictionary instance okay */
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dictionary_find(&test_dictionary, &predicate, &cursor));

	/* check the status of the cursor as it should be initialized */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	/* user must allocate memory before calling next() */
	ion_record_t record;

	record.value	= malloc(record_info.value_size);
	record.key		= malloc(record_info.key_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == cursor->next(cursor, &record));

	/* check that value is correct that has been returned */
	ion_value_t str;

	str = malloc(record_info.value_size);
	sprintf((char *) str, "value : %i", *(int *) predicate.statement.equality.equality_value);

	PLANCK_UNIT_ASSERT_TRUE(tc, ION_IS_EQUAL == memcmp(record.value, str, record_info.value_size));

	free(str);

	/* and as there is only 1 result, the next call should return empty */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->next(cursor, &record));

	/* and as there is only 1 result, the next call should return empty */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->next(cursor, &record));

	free(record.value);
	free(record.key);
	/* destory cursor for cleanup */
	cursor->destroy(&cursor);
	/* and destroy the dictionary instance */
	dictionary_delete_dictionary(&test_dictionary);
}

void
test_open_address_dictionary_handler_query_no_results(
	planck_unit_test_t *tc
) {
	int					size;
	ion_record_info_t	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size	= sizeof(int);
	record_info.value_size	= 10;
	size					= 10;

	ion_dictionary_handler_t	map_handler;			/* create handler for hashmap */
	ion_dictionary_t			test_dictionary;		/* dictionary handler for test instance */

	createTestDictionary(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	ion_dict_cursor_t *cursor;	/* create a new cursor pointer */

	/* create a new predicate statement */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(-1, int));

	/* test that the query runs on dictionary instance okay */
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dictionary_find(&test_dictionary, &predicate, &cursor));

	/* check the status of the cursor as it should be at the end of results as no values exist */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

	/* user must allocate memory before calling next() */
	ion_record_t record;

	record.value	= malloc(record_info.value_size);
	record.key		= malloc(record_info.key_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->next(cursor, &record));

	free(record.value);
	free(record.key);
	/* destroy cursor for cleanup */
	cursor->destroy(&cursor);
	/* and destroy the dictionary instance */
	dictionary_delete_dictionary(&test_dictionary);
}

void
test_open_address_dictionary_predicate_equality(
	planck_unit_test_t *tc
) {
	ion_key_t key_under_test;

	key_under_test = malloc(sizeof(int));

	int					size;
	ion_record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	ion_dictionary_handler_t	map_handler;			/* create handler for hashmap */
	ion_dictionary_t			test_dictionary;		/* dictionary handler for test instance */

	createTestDictionary(&map_handler, &record, size, &test_dictionary, key_type_numeric_signed);

	ion_dict_cursor_t cursor;	/* create a new cursor */

	cursor.destroy = oadict_destroy_cursor;

	/* create a new predicate statement */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1, int));

	cursor.dictionary	= &test_dictionary;					/* register test dictionary */
	cursor.predicate	= &predicate;						/* register predicate */

	memcpy(key_under_test, &(int) { 1 }, sizeof(int));

	/* printf("key %i\n",*(int *)key_under_test); */

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(int) { 2 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(int) { -1 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == test_predicate(&cursor, key_under_test));

	free(key_under_test);

	/* destroy cursor for cleanup */
	/* cursor->destroy(&cursor); */
	/* and destroy the dictionary instance */
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_dictionary_predicate_range_signed(
	planck_unit_test_t *tc
) {
	ion_key_t key_under_test;

	key_under_test = malloc(sizeof(int));

	int					size;
	ion_record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	ion_dictionary_handler_t	map_handler;			/* create handler for hashmap */
	ion_dictionary_t			test_dictionary;		/* dictionary handler for test instance */

	createTestDictionary(&map_handler, &record, size, &test_dictionary, key_type_numeric_signed);

	ion_dict_cursor_t cursor;	/* create a new cursor */

	cursor.destroy = oadict_destroy_cursor;

	/* create a new predicate statement */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(-1, int), IONIZE(1, int));

	cursor.dictionary	= &test_dictionary;					/* register test dictionary */
	cursor.predicate	= &predicate;						/* register predicate */

	memcpy(key_under_test, &(int) { 0 }, sizeof(int));

	/* printf("key %i\n",*(int *)key_under_test); */

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(int) { -1 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(int) { 1 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(int) { 2 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(int) { -2 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == test_predicate(&cursor, key_under_test));

	free(key_under_test);

	/* destroy cursor for cleanup */
	/* cursor->destroy(&cursor); */
	/* and destroy the dictionary instance */
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_dictionary_predicate_range_unsigned(
	planck_unit_test_t *tc
) {
	ion_key_t key_under_test;

	key_under_test = malloc(sizeof(unsigned int));

	int					size;
	ion_record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	ion_dictionary_handler_t	map_handler;			/* create handler for hashmap */
	ion_dictionary_t			test_dictionary;		/* dictionary handler for test instance */

	createTestDictionary(&map_handler, &record, size, &test_dictionary, key_type_numeric_unsigned);

	ion_dict_cursor_t cursor;	/* create a new cursor */

	cursor.destroy = oadict_destroy_cursor;

	/* create a new predicate statement */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(0, int), IONIZE(2, int));

	cursor.dictionary	= &test_dictionary;					/* register test dictionary */
	cursor.predicate	= &predicate;						/* register predicate */

	memcpy(key_under_test, &(unsigned int) { 0 }, sizeof(unsigned int));

	/* printf("key %i\n",*(unsigned int *)key_under_test); */

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(unsigned int) { 1 }, sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(unsigned int) { 2 }, sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(unsigned int) { 3 }, sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == test_predicate(&cursor, key_under_test));

	memcpy(key_under_test, &(unsigned int) { 4 }, sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == test_predicate(&cursor, key_under_test));

	free(key_under_test);

	/* destroy cursor for cleanup */
	/* cursor->destroy(&cursor); */
	/* and destroy the dictionary instance */
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_dictionary_cursor_range(
	planck_unit_test_t *tc
) {
	int					size;
	ion_record_info_t	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size	= sizeof(int);
	record_info.value_size	= 10;
	size					= 10;

	ion_dictionary_handler_t	map_handler;			/* create handler for hashmap */
	ion_dictionary_t			test_dictionary;		/* dictionary handler for test instance */

	createTestDictionary(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	ion_dict_cursor_t *cursor;	/* create a new cursor */

	/* create a new predicate statement */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(1, int), IONIZE(5, int));
	/* test that the query runs on dictionary instance okay */
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dictionary_find(&test_dictionary, &predicate, &cursor));

	/* check the status of the cursor as it should be initialized */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	/* user must allocate memory before calling next() */
	ion_record_t record;

	record.value	= malloc(record_info.value_size);
	record.key		= malloc(record_info.key_size);

	int					result_count = 0;
	ion_cursor_status_t cursor_status;

	while (cs_cursor_active == (cursor_status = cursor->next(cursor, &record))) {
		PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == cursor_status);

		/* check that value is correct that has been returned */
		ion_value_t str;

		str = malloc(record_info.value_size);
		sprintf((char *) str, "value : %i", (*(int *) predicate.statement.range.lower_bound) + result_count);

		PLANCK_UNIT_ASSERT_TRUE(tc, ION_IS_EQUAL == memcmp(record.value, str, record_info.value_size));
		PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) (record.key) >= *(int *) (cursor->predicate->statement.range.lower_bound));
		PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) (record.key) <= *(int *) (cursor->predicate->statement.range.upper_bound));
		result_count++;
		free(str);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, 5 == result_count);

	/* and as there is only 1 result, the next call should return empty */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->next(cursor, &record));

	/* destroy the cursor */
	cursor->destroy(&cursor);

	/* and check that cursor has been destroyed correctly */
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	free(record.key);
	free(record.value);

	/* and destroy the dictionary instance */
	dictionary_delete_dictionary(&test_dictionary);
}

planck_unit_suite_t *
open_address_hashmap_handler_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_hashmap_handler_function_registration);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_hashmap_handler_create_destroy);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_dictionary_predicate_equality);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_dictionary_predicate_range_signed);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_dictionary_predicate_range_unsigned);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_dictionary_cursor_equality);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_dictionary_handler_query_with_results);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_dictionary_handler_query_no_results);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_dictionary_cursor_range);

	return suite;
}

void
runalltests_open_address_hash_handler(
) {
	/* CuString	*output	= CuStringNew(); */
	planck_unit_suite_t *suite = open_address_hashmap_handler_getsuite();

	planck_unit_run_suite(suite);
	/* CuSuiteSummary(suite, output); */
	/* CuSuiteDetails(suite, output); */
	/* printf("%s\n", output->buffer); */

	planck_unit_destroy_suite(suite);
	/* CuSuiteDelete(suite); */
	/* CuStringDelete(output); */
}
