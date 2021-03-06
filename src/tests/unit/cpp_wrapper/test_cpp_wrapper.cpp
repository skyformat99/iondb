/******************************************************************************/
/**
@file		test_cpp_wrapper.cpp
@author		Eric Huang, Dana Klamut
@brief		Unit test for the C++ Wrapper
@todo		These tests need to be fully fleshed out.
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

#include "../../planck-unit/src/planck_unit.h"
#include "../../../cpp_wrapper/Dictionary.h"
#include "../../../cpp_wrapper/MasterTable.h"
#include "test_cpp_wrapper.h"

/* This is used to define how complicated to pre-fill a dictionary for testing. */
typedef enum ION_BEHAVIOUR_FILL_LEVEL {
	ion_fill_none, ion_fill_low, ion_fill_medium, ion_fill_high, ion_fill_edge_cases
} ion_behaviour_fill_level_e;

#define ION_FILL_LOW_LOOP(var) \
	for (var = 0; var<10;var++)
#define ION_FILL_MEDIUM_LOOP(var) \
	for (var = 50; var<100;var += 2)
#define ION_FILL_HIGH_LOOP(var) \
	for (var = 500; var<1000;var += 5)
#define ION_FILL_EDGE_LOOP(var) \
	for (var = -100; var<-50;var += 2)

#define NULL_VALUE 999	/** Value arbitrarily chosen to represent NULL */

/**
@brief	This function initializes the master table.
*/
void
master_table_init(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
) {
	ion_err_t err = master_table->initializeMasterTable();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);
}

/**
@brief	This function closes the master table.
*/
void
master_table_close(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
) {
	ion_err_t err = master_table->closeMasterTable();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);
}

/**
@brief	This function performs cleanup if the master table was not previously
		destroyed properly.
*/
void
master_table_setup(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
) {
	master_table_close(tc, master_table);
	fremove(ION_MASTER_TABLE_FILENAME);
}

/**
@brief	This function performs the dictionary initialization using the master table.
*/
void
master_table_dictionary_add(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary,
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	ion_err_t err = master_table->addToMasterTable(dictionary, dictionary_size);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_type, dictionary->dict.instance->key_type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_size, dictionary->dict.instance->record.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value_size, dictionary->dict.instance->record.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type, dictionary->dict.instance->type);
}

/**
@brief	Deletes the tracking of a dictionary from the master table.
*/
void
master_table_delete_from_master_table(
	planck_unit_test_t	*tc,
	MasterTable			*master_table,
	ion_dictionary_id_t id
) {
	ion_err_t err = master_table->deleteFromMasterTable(id);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	ion_dictionary_config_info_t config;

	err = master_table->lookupMasterTable(id, &config);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, err);
}

/**
@brief	This function opens a dictionary using the master table.
*/
void
master_table_open_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_id_t id
) {
	ion_err_t err = master_table->openDictionary(dictionary, id);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function closes a dictionary using the master table.
*/
void
master_table_close_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary
) {
	ion_err_t err = master_table->closeDictionary(dictionary);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_closed, dictionary->dict.status);
}

/**
@brief	This function looks up a dictionary in the master table using the
		dictionary's ID and configurations.
*/
void
master_table_lookup_dictionary(
	planck_unit_test_t				*tc,
	MasterTable						*master_table,
	ion_dictionary_id_t				id,
	ion_key_type_t					key_type,
	ion_key_size_t					key_size,
	ion_value_size_t				value_size,
	ion_dictionary_size_t			dictionary_size,
	ion_dictionary_type_t			dictionary_type,
	ion_dictionary_config_info_t	*config,
	ion_boolean_t					expect_found
) {
	ion_err_t err = master_table->lookupMasterTable(id, config);

	if (expect_found) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, id, config->id);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_type, config->type);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_size, config->key_size);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value_size, config->value_size);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_size, config->dictionary_size);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type, config->dictionary_type);
	}
	else {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, err);
	}
}

/**
@brief	This function creates a dictionary using the master table.
*/
void
master_table_create_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_id_t id,
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, id, dictionary->dict.instance->id);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_type, dictionary->dict.instance->key_type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_size, dictionary->dict.instance->record.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value_size, dictionary->dict.instance->record.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_size, dictionary->dict_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type, dictionary->dict.instance->type);

	ion_dictionary_config_info_t config;

	master_table_lookup_dictionary(tc, master_table, id, key_type, key_size, value_size, dictionary_size, dictionary_type, &config, boolean_true);
}

/**
@brief	This function deletes a dictionary using the master table.
*/
void
master_table_delete_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary
) {
	ion_dictionary_id_t id	= dictionary->dict.instance->id;

	ion_err_t err			= master_table->deleteDictionary(dictionary);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	ion_dictionary_config_info_t config;

	err = master_table->lookupMasterTable(id, &config);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, err);
}

/**
@brief	This function performs a get on a dictionary.
*/
void
cpp_wrapper_get(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int expected_value,
	ion_err_t expected_status,
	ion_result_count_t expected_count
) {
	int				retval	= dict->get(key);
	ion_status_t	status	= dict->last_status;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (err_ok == status.error) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_value, retval);
	}
}

/**
@brief	This function does an insert into a dictionary.
*/
void
cpp_wrapper_insert(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int value,
	ion_boolean_t check_result
) {
	ion_status_t status = dict->insert(key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	if (check_result) {
		cpp_wrapper_get(tc, dict, key, value, err_ok, 1);
	}
}

/**
@brief	This function performs a delete on a dictionary.
*/
void
cpp_wrapper_delete(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	ion_err_t expected_status,
	ion_result_count_t expected_count,
	ion_boolean_t check_result
) {
	ion_status_t status = dict->deleteRecord(key);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		cpp_wrapper_get(tc, dict, key, NULL_VALUE, err_item_not_found, 0);
	}
}

/**
@brief	This function performs an update on a dictionary.
*/
void
cpp_wrapper_update(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int value,
	ion_err_t expected_status,
	ion_result_count_t expected_count,
	ion_boolean_t check_result
) {
	ion_status_t status = dict->update(key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		cpp_wrapper_get(tc, dict, key, value, err_ok, 1);
	}
}

/**
@brief	This function performs the setup required for a test case.
*/
void
cpp_wrapper_setup(
	planck_unit_test_t *tc,
	Dictionary<int, int>			*dict,
	ion_behaviour_fill_level_e fill_level
) {
	/* This switch statement intentionally doesn't have breaks - we want it to fall through. */
	int i;

	switch (fill_level) {
		case ion_fill_edge_cases: {
			ION_FILL_EDGE_LOOP(i) {
				cpp_wrapper_insert(tc, dict, i, i * 3, boolean_true);
			}
		}

		case ion_fill_high: {
			ION_FILL_HIGH_LOOP(i) {
				cpp_wrapper_insert(tc, dict, i, i * 10, boolean_true);
			}
		}

		case ion_fill_medium: {
			ION_FILL_MEDIUM_LOOP(i) {
				cpp_wrapper_insert(tc, dict, i, i * 5, boolean_true);
			}
		}

		case ion_fill_low: {
			ION_FILL_LOW_LOOP(i) {
				cpp_wrapper_insert(tc, dict, i, i * 2, boolean_true);
			}
		}

		case ion_fill_none: {
			/* Intentionally left blank */
		}
	}
}

/**
@brief	This function performs an open and close on a dictionary.
*/
void
cpp_wrapper_open_close(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key,
	int value
) {
	ion_err_t				error;
	ion_dictionary_id_t		gdict_id	= dict->dict.instance->id;
	ion_key_size_t			key_size	= dict->dict.instance->record.key_size;
	ion_value_size_t		val_size	= dict->dict.instance->record.value_size;
	ion_key_type_t			key_type	= dict->dict.instance->key_type;
	ion_dictionary_size_t	dict_size	= dict->dict_size;
	ion_dictionary_type_t	dict_type	= dict->dict.instance->type;

	/* Insert test record so we can check data integrity after we close/open */
	cpp_wrapper_insert(tc, dict, key, value, boolean_false);

	/* Check the test record */
	cpp_wrapper_get(tc, dict, key, value, err_ok, 1);

	error = dict->close();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	ion_dictionary_config_info_t config = {
		gdict_id, 0, key_type, key_size, val_size, dict_size, dict_type
	};

	error = dict->open(config);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dict->dict.instance->record.key_size, key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dict->dict.instance->record.value_size, val_size);

	/* Check the test record */
	cpp_wrapper_get(tc, dict, key, value, err_ok, 1);
}

/**
@brief	This function performs an equality cursor query on a dictionary.
*/
void
cpp_wrapper_equality(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key,
	int expected_values[],
	int expected_num_records,
	ion_boolean_t key_exists
) {
	Cursor<int, int> *cursor = dict->equality(key);

	if (!key_exists) {
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->next());

		delete cursor;

		return;
	}

	int records_found	= 0;
	int curr_pos		= 0;

	PLANCK_UNIT_ASSERT_TRUE(tc, cursor->hasNext());

	ion_cursor_status_t status = cursor->next();

	while (status) {
		for (int i = 0; i < key; i++) {
			if (expected_values[i] == cursor->getValue()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key, cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_values[curr_pos], cursor->getValue());

		expected_values[curr_pos]	= NULL_VALUE;
		status						= cursor->next();
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_num_records, records_found);

	delete cursor;
}

/**
@brief	This function performs a range cursor query on a dictionary.
*/
void
cpp_wrapper_range(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int min_key,
	int max_key,
	int expected_records[],
	int expected_num_records,
	ion_boolean_t records_exist
) {
	PLANCK_UNIT_ASSERT_TRUE(tc, min_key < max_key);

	Cursor<int, int> *cursor = dict->range(min_key, max_key);

	if (!records_exist) {
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->next());

		delete cursor;

		return;
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cursor->hasNext());

	int records_found			= 0;
	int curr_pos				= 0;

	ion_cursor_status_t status	= cursor->next();

	while (status) {
		for (int i = 0; i < expected_num_records; i++) {
			if (expected_records[i] == cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_records[curr_pos], cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_records[curr_pos], cursor->getValue());

		expected_records[curr_pos]	= NULL_VALUE;
		status						= cursor->next();
		min_key++;
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_num_records, records_found);

	delete cursor;
}

/**
@brief	This function performs an all records cursor query on a dictionary.
*/
void
cpp_wrapper_all_records(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int expected_records[],
	int expected_num_records,
	ion_boolean_t records_exist
) {
	Cursor<int, int> *cursor = dict->allRecords();

	if (!records_exist) {
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->next());

		delete cursor;

		return;
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cursor->hasNext());

	int records_found			= 0;
	int curr_pos				= 0;

	ion_cursor_status_t status	= cursor->next();

	while (status) {
		for (int i = 0; i < expected_num_records; i++) {
			if (expected_records[i] == cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_records[curr_pos], cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_records[curr_pos], cursor->getValue());

		expected_records[curr_pos]	= NULL_VALUE;
		status						= cursor->next();
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_num_records, records_found);

	delete cursor;
}

/* =================================================== TEST CASES =================================================== */

/**
@brief	This function tests whether or not we can build a dictionary.
*/
void
test_cpp_wrapper_setup(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
}

/**
@brief	This function tests whether or not we can build and a dictionary
		on all dictionary implementations.
*/
void
test_cpp_wrapper_setup_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;
}

/**
@brief	This function tests a single insertion into a dictionary.
*/
void
test_cpp_wrapper_insert_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 10, 20, boolean_false);
}

/**
@brief	This function tests a single insertion into a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_insert_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests multiple insertions into a dictionary.
*/
void
test_cpp_wrapper_insert_multiple(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);

	int i;

	for (i = 50; i < 55; i++) {
		cpp_wrapper_insert(tc, dict, i, i * 2, boolean_false);
	}
}

/**
@brief	This function tests multiple insertions into a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_insert_multiple_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;
}

/**
@brief	This function tests a retrieval on a dictionary that only has one record in it.
*/
void
test_cpp_wrapper_get_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 99, 99 * 2, boolean_true);
}

/**
@brief	This function tests a retrieval on a dictionary that only has one record
		in it on all dictionary implementations.
*/
void
test_cpp_wrapper_get_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests retrieval on a dictionary with many records in it.
*/
void
test_cpp_wrapper_get_in_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 35, 35 * 2, boolean_true);
	cpp_wrapper_insert(tc, dict, 1002, 1002 * 2, boolean_true);
	cpp_wrapper_insert(tc, dict, 55, 55 * 2, boolean_true);
	cpp_wrapper_insert(tc, dict, -5, -5 * 2, boolean_true);
}

/**
@brief	This function tests retrieval on a dictionary with many records in it
		on all dictionary implementations.
*/
void
test_cpp_wrapper_get_in_many_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;
}

/**
@brief	This function tests retrieval on a dictionary with a whole bunch of records in it.
*/
void
test_cpp_wrapper_get_lots(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);

	int i;

	for (i = 300; i < 1000; i += 15) {
		cpp_wrapper_insert(tc, dict, i, i * 5, boolean_true);
	}

	for (i = 300; i < 1000; i += 15) {
		cpp_wrapper_get(tc, dict, i, i * 5, err_ok, 1);
	}
}

/**
@brief	This function tests retrieval on a dictionary with a whole bunch of
		records in it on all dictionary implementations.
*/
void
test_cpp_wrapper_get_lots_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;
}

/**
@brief	This function tests retrieval on an empty dictionary for a key that doesn't exist.
*/
void
test_cpp_wrapper_get_nonexist_empty(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_get(tc, dict, 99, NULL_VALUE, err_item_not_found, 0);
}

/**
@brief	This function tests retrieval on an empty dictionary for a key that
		doesn't exist on all dictionary implementations.
*/
void
test_cpp_wrapper_get_nonexist_empty_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;
}

/**
@brief	This function tests retrieval on a dictionary for a single key that doesn't exist.
*/
void
test_cpp_wrapper_get_nonexist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_low);
	cpp_wrapper_get(tc, dict, 99, NULL_VALUE, err_item_not_found, 0);
}

/**
@brief	This function tests retrieval on a dictionary for a single key that
		doesn't exist on all dictionary implementations.
*/
void
test_cpp_wrapper_get_nonexist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests retrieval on a dictionary for many keys that don't exist.
*/
void
test_cpp_wrapper_get_nonexist_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);
	cpp_wrapper_get(tc, dict, -2000, NULL_VALUE, err_item_not_found, 0);
	cpp_wrapper_get(tc, dict, 3000, NULL_VALUE, err_item_not_found, 0);
}

/**
@brief	This function tests retrieval on a dictionary for many keys that don't
		exist on all dictionary implementations.
*/
void
test_cpp_wrapper_get_nonexist_many_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;
}

/**
@brief	This function tests retrieval on a dictionary that has one record in it.
		We search for a key that exists, and expect that we get a positive response back.
*/
void
test_cpp_wrapper_get_exist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 30, 30, boolean_true);
	cpp_wrapper_get(tc, dict, 30, 30, err_ok, 1);
}

/**
@brief	This function tests retrieval on a dictionary that has one record in it
		on all dictionary implementations. We search for a key that exists, and
		expect that we get a positive response back.
*/
void
test_cpp_wrapper_get_exist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it.
		We expect a positive result.
*/
void
test_cpp_wrapper_get_populated_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_low);
	cpp_wrapper_insert(tc, dict, 92, 92, boolean_true);
	cpp_wrapper_get(tc, dict, 92, 92, err_ok, 1);
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it
		on all dictionary implementations. We expect a positive result.
*/
void
test_cpp_wrapper_get_populated_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it.
		We expect a positive result on all gets run.
*/
void
test_cpp_wrapper_get_populated_multiple(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_low);

	int i;

	ION_FILL_LOW_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 2, err_ok, 1);
	}
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it
		on all dictionary implementations. We expect a positive result on all
		gets run.
*/
void
test_cpp_wrapper_get_populated_multiple_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;
}

/**
@brief	This function tests a get of everything within a dictionary.
*/
void
test_cpp_wrapper_get_all(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 2, err_ok, 1);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 5, err_ok, 1);
	}
	ION_FILL_HIGH_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 10, err_ok, 1);
	}
	ION_FILL_EDGE_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 3, err_ok, 1);
	}
}

/**
@brief	This function tests a get of everything within a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_get_all_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;
}

/**
@brief	This function tests deletion on an empty dictionary.
		We expect to receive err_item_not_found and for everything to remain as-is.
*/
void
test_cpp_wrapper_delete_empty(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_delete(tc, dict, 3, err_item_not_found, 0, boolean_true);
}

/**
@brief	This function tests deletion on an empty dictionary on all dictionary
		implementations. We expect to receive err_item_not_found and for everything
		else to remain as-is.
*/
void
test_cpp_wrapper_delete_empty_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;
}

/**
@brief	This function tests deletion on a dictionary that has one element,
		but not the one we are looking for. We expect to receive err_item_not_found
		and for everything to remain as-is.
*/
void
test_cpp_wrapper_delete_nonexist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 5, 10, boolean_true);
	cpp_wrapper_delete(tc, dict, 3, err_item_not_found, 0, boolean_true);
}

/**
@brief	This function tests deletion on a dictionary that has one element, but not
		the one we are looking for on all dictionary implementations. We expect to
		receive err_item_not_found and for everything else to remain as-is.
*/
void
test_cpp_wrapper_delete_nonexist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests deletion on a dictionary that has many elements,
		but not the one we are looking for. We expect to receive err_item_not_found
		and for everything to remain as-is.
*/
void
test_cpp_wrapper_delete_nonexist_several(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_medium);
	cpp_wrapper_delete(tc, dict, -100, err_item_not_found, 0, boolean_true);
}

/**
@brief	This function tests deletion on a dictionary that has many elements, but not
		the one we are looking for on all dictionary implementations. We expect to
		receive err_item_not_found and for everything else to remain as-is.
*/
void
test_cpp_wrapper_delete_nonexist_several_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;
}

/**
@brief	This function tests deletion on a dictionary that has a single element,
		which is the one we're looking for.
*/
void
test_cpp_wrapper_delete_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 3, 6, boolean_true);
	cpp_wrapper_delete(tc, dict, 3, err_ok, 1, boolean_true);
}

/**
@brief	This function tests deletion on a dictionary that has a single element,
		which is the one we are looking for on all dictionary implementations.
*/
void
test_cpp_wrapper_delete_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests deletion on a dictionary that has many elements,
		of which contains the one we choose to delete.
*/
void
test_cpp_wrapper_delete_single_several(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_high);
	cpp_wrapper_delete(tc, dict, 700, err_ok, 1, boolean_true);
}

/**
@brief	This function tests deletion on a dictionary that has many elements, of
		which contains the one we choose to delete on all dictionary implementations.
*/
void
test_cpp_wrapper_delete_single_several_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;
}

/**
@brief	This function tests deletion of everything within a dictionary.
*/
void
test_cpp_wrapper_delete_all(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		cpp_wrapper_delete(tc, dict, i, err_ok, 1, boolean_true);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		cpp_wrapper_delete(tc, dict, i, err_ok, 1, boolean_true);
	}
	ION_FILL_HIGH_LOOP(i) {
		cpp_wrapper_delete(tc, dict, i, err_ok, 1, boolean_true);
	}
	ION_FILL_EDGE_LOOP(i) {
		cpp_wrapper_delete(tc, dict, i, err_ok, 1, boolean_true);
	}
}

/**
@brief	This function tests deletion of everything within a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_delete_all_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;
}

/**
@brief	This function tests an update on a dictionary that is empty. We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_empty_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_update(tc, dict, 3, 5, err_ok, 1, boolean_true);
}

/**
@brief	This function tests an update on a dictionary that is empty on all dictionary
		implementations. We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_empty_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests an update on a dictionary that has one element,
		but not the one we are looking for. We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_nonexist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 10, 4, boolean_true);
	cpp_wrapper_update(tc, dict, 3, 5, err_ok, 1, boolean_true);
}

/**
@brief	This function tests an update on a dictionary that has one element,
		but not the one we are looking for on all dictionary implementations.
		We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_nonexist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		but not the one we are looking for. We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_nonexist_in_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_medium);
	cpp_wrapper_update(tc, dict, 63, -10, err_ok, 1, boolean_true);
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		but not the one we are looking for on all dictionary implementations.
		We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_nonexist_in_many_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;
}

/**
@brief	This function tests an update on a dictionary that has one element,
		which is the one we are looking for.
*/
void
test_cpp_wrapper_update_exist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 23, 0, boolean_true);
	cpp_wrapper_update(tc, dict, 23, 44, err_ok, 1, boolean_true);
}

/**
@brief	This function tests an update on a dictionary that has one element,
		which is the one we are looking for on all dictionary implementations.
*/
void
test_cpp_wrapper_update_exist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		which includes the one we are looking for.
*/
void
test_cpp_wrapper_update_exist_in_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_medium);
	cpp_wrapper_update(tc, dict, 60, -23, err_ok, 1, boolean_true);
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		which includes the one we are looking for on all dictionary implementations.
*/
void
test_cpp_wrapper_update_exist_in_many_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;
}

/**
@brief	This function tests update of everything within a dictionary.
*/
void
test_cpp_wrapper_update_all(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		cpp_wrapper_update(tc, dict, i, -1337, err_ok, 1, boolean_true);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		cpp_wrapper_update(tc, dict, i, -1337, err_ok, 1, boolean_true);
	}
	ION_FILL_HIGH_LOOP(i) {
		cpp_wrapper_update(tc, dict, i, -1337, err_ok, 1, boolean_true);
	}
	ION_FILL_EDGE_LOOP(i) {
		cpp_wrapper_update(tc, dict, i, -1337, err_ok, 1, boolean_true);
	}
}

/**
@brief	This function tests an update of everything within a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_update_all_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;
}

/**
@brief	This function tests some deletes, followed by an insert. Nothing should go wrong.
*/
void
test_cpp_wrapper_delete_then_insert(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);

	cpp_wrapper_delete(tc, dict, 60, err_ok, 1, boolean_true);
	cpp_wrapper_delete(tc, dict, 4, err_ok, 1, boolean_true);
	cpp_wrapper_delete(tc, dict, 505, err_ok, 1, boolean_true);

	cpp_wrapper_insert(tc, dict, 61, 44, boolean_true);
	cpp_wrapper_insert(tc, dict, 67, 42, boolean_true);
	cpp_wrapper_insert(tc, dict, 73, 48, boolean_true);
}

/**
@brief	This function tests some deletes, followed by an insert on all dictionary
		implementations. Nothing should go wrong.
*/
void
test_cpp_wrapper_delete_then_insert_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		on implementations that support duplicate keys.
*/
void
test_cpp_wrapper_equality_duplicates(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key
) {
	int values[key];

	for (int i = 1; i <= key; i++) {
		cpp_wrapper_insert(tc, dict, key, i * 2, boolean_false);
		values[i - 1] = i * 2;

		cpp_wrapper_insert(tc, dict, i - 1, (i - 1) * 2, boolean_false);
	}

	cpp_wrapper_equality(tc, dict, key, values, key, boolean_true);
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		on implementations that do not allow duplicate keys.
*/
void
test_cpp_wrapper_equality_no_duplicates(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key
) {
	for (int i = 0; i < key + 3; i++) {
		cpp_wrapper_insert(tc, dict, i, i * 2, boolean_false);
	}

	int values[1] = { key * 2 };

	cpp_wrapper_equality(tc, dict, key, values, 1, boolean_true);
}

/**
@brief	Aggregate test to test equality cursor query on all dictionary implementations.
*/
void
test_cpp_wrapper_equality(
	planck_unit_test_t	*tc,
	int					key
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_equality_duplicates(tc, dict, key);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_equality_duplicates(tc, dict, key);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, key);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, key);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, key);
	delete dict;
}

/**
@brief	Aggregate test to test equality cursor query on all dictionary implementations.
*/
void
test_cpp_wrapper_equality_all(
	planck_unit_test_t *tc
) {
	test_cpp_wrapper_equality(tc, 10);
	test_cpp_wrapper_equality(tc, 4);
	test_cpp_wrapper_equality(tc, 3);
	test_cpp_wrapper_equality(tc, 6);
}

/**
@brief	This function tests an equality cursor query on an empty dictionary for a key
		that doesn't exist.
*/
void
test_cpp_wrapper_equality_nonexist_empty(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int values[1] = { 0 };

	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_equality(tc, dict, NULL_VALUE, values, 0, boolean_false);
}

/**
@brief	Aggregate test to test equality cursor query on an empty dictionary
		for a key that doesn't exist on all dictionary implementations.
*/
void
test_cpp_wrapper_equality_nonexist_empty_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_equality_nonexist_empty(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_equality_nonexist_empty(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_equality_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_nonexist_empty(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		when the desired key is not present.
*/
void
test_cpp_wrapper_equality_nonexist(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	for (int i = 0; i < 10; i++) {
		cpp_wrapper_insert(tc, dict, i, i * 2, boolean_false);
	}

	int values[1] = { NULL_VALUE };

	cpp_wrapper_equality(tc, dict, 20, values, 0, boolean_false);
}

/**
@brief	Aggregate test to test equality cursor query when the desired key is not
		present on all dictionary implementations.
*/
void
test_cpp_wrapper_equality_nonexist_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_equality_nonexist(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_equality_nonexist(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_equality_nonexist(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_nonexist(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_nonexist(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform a range cursor query
		for any desired range of keys.
*/
void
test_cpp_wrapper_range(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int min_key,
	int max_key
) {
	/* records_expected is calculated the following way because it is min_key and max_key inclusive. */
	int records_expected	= max_key - min_key + 1;
	int curr_pos			= 0;
	int nums[records_expected];

	for (int i = 0; i < max_key + 5; i++) {
		cpp_wrapper_insert(tc, dict, i, i, boolean_true);

		if ((i >= min_key) && (i <= max_key)) {
			nums[curr_pos] = i;
			curr_pos++;
		}
	}

	cpp_wrapper_range(tc, dict, min_key, max_key, nums, records_expected, boolean_true);
}

/**
@brief	Aggregate test to test small range cursor query on all dictionary implementations.
*/
void
test_cpp_wrapper_range_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range(tc, dict, 5, 6);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range(tc, dict, 1, 2);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range(tc, dict, 17, 18);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range(tc, dict, 30, 31);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range(tc, dict, 6, 7);
	delete dict;
}

/**
@brief	Aggregate test to test large range cursor query on all dictionary implementations.
*/
void
test_cpp_wrapper_range_multiple_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range(tc, dict, 5, 10);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range(tc, dict, 1, 7);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range(tc, dict, 13, 18);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range(tc, dict, 30, 39);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range(tc, dict, 6, 23);
	delete dict;
}

/**
@brief	Tests a range cursor on an empty dictionary where clearly no keys within
		the range exist.
*/
void
test_cpp_wrapper_range_nonexist_empty(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int values[1] = { NULL_VALUE };

	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_range(tc, dict, 0, 50, values, 0, boolean_false);
}

/**
@brief	Aggregate test to test a range cursor query on an empty dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_range_nonexist_empty_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	/* This test has been excluded as the BppTree range query functionality */
	/* needs to be revised. */

/*	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int)); */
/*	test_cpp_wrapper_range_nonexist_empty(tc, dict); */
/*	delete dict; */

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range_nonexist_empty(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_range_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_range_nonexist_empty(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform a range cursor query
		on range values not present in dictionary.
*/
void
test_cpp_wrapper_range_nonexist(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int records[5]		= { 3, 10, 25, 50, 70 };
	int records_length	= sizeof(records) / sizeof(int);

	for (int i = 0; i < records_length; i++) {
		cpp_wrapper_insert(tc, dict, records[i], records[i], boolean_true);
	}

	cpp_wrapper_range(tc, dict, -1000, 0, records, 0, boolean_false);
}

/**
@brief	Aggregate test to test range cursor query of nonexistent values on all
		implementations.
*/
void
test_cpp_wrapper_range_nonexist_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range_nonexist(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range_nonexist(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range_nonexist(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range_nonexist(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range_nonexist(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		where the desired records are in the middle of the dictionary.
*/
void
test_cpp_wrapper_range_exist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int records[5]			= { 3, 10, 25, 50, 70 };
	int expected_records[2] = { 25, 50 };
	int records_length		= sizeof(records) / sizeof(int);

	for (int i = 0; i < records_length; i++) {
		cpp_wrapper_insert(tc, dict, records[i], records[i], boolean_true);
	}

	cpp_wrapper_range(tc, dict, 15, 55, expected_records, 2, boolean_true);
}

/**
@brief	Aggregate test to test small range cursor query on all dictionary implementations.
*/
void
test_cpp_wrapper_range_exist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range_exist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range_exist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range_exist_single(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		where range query includes all records present in the dictionary.
*/
void
test_cpp_wrapper_range_all(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int records[5]		= { 1, 2, 3, 4, 5 };
	int records_length	= sizeof(records) / sizeof(int);

	for (int i = 0; i < records_length; i++) {
		cpp_wrapper_insert(tc, dict, records[i], records[i], boolean_true);
	}

	cpp_wrapper_range(tc, dict, 1, 5, records, records_length, boolean_true);
}

/**
@brief	Aggregate test to test range cursor query of entire dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_range_all_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range_all(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range_all(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range_all(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range_all(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range_all(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform an all records cursor query.
*/
void
test_cpp_wrapper_all_records(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int random_positive_num
) {
	int records[random_positive_num];

	for (int i = 0; i < random_positive_num; i++) {
		cpp_wrapper_insert(tc, dict, i, i, boolean_true);
		records[i] = i;
	}

	cpp_wrapper_all_records(tc, dict, records, random_positive_num, boolean_true);
}

/**
@brief	Aggregate test to test an all records cursor query on all dictionary
		implementations.
*/
void
test_cpp_wrapper_all_records_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records(tc, dict, 10);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_all_records(tc, dict, 4);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_all_records(tc, dict, 13);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_all_records(tc, dict, 5);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_all_records(tc, dict, 8);
	delete dict;
}

/**
@brief	Tests an all records cursor on an empty dictionary.
*/
void
test_cpp_wrapper_all_records_nonexist_empty(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int values[1] = { NULL_VALUE };

	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_all_records(tc, dict, values, 0, boolean_false);
}

/**
@brief	Aggregate test to test an all records cursor query of an empty dictionary
		on all dictionary implementations.
*/
void
test_cpp_wrapper_all_records_nonexist_empty_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records_nonexist_empty(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_all_records_nonexist_empty(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_all_records_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_all_records_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_all_records_nonexist_empty(tc, dict);
	delete dict;
}

/**
@brief	Tests an all records cursor query where the keys are sparsely inserted.
*/
void
test_cpp_wrapper_all_records_populated(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int records[5]		= { 3, 10, 25, 50, 70 };
	int records_length	= sizeof(records) / sizeof(int);

	for (int i = 0; i < records_length; i++) {
		cpp_wrapper_insert(tc, dict, records[i], records[i], boolean_true);
	}

	cpp_wrapper_all_records(tc, dict, records, records_length, boolean_true);
}

/**
@brief	Aggregate test to test all records cursor query of a sparsely populated
		dictionary on all dictionary implementations.
*/
void
test_cpp_wrapper_all_records_populated_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records_populated(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_all_records_populated(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_all_records_populated(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_all_records_populated(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_all_records_populated(tc, dict);
	delete dict;
}

/**
@brief	Tests an all records cursor query where keys are not inserted in
		increasing order and include a wide range.
*/
void
test_cpp_wrapper_all_records_random(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int records[5]		= { -111, 113, 5, 50, -225 };
	int records_length	= sizeof(records) / sizeof(int);

	for (int i = 0; i < records_length; i++) {
		cpp_wrapper_insert(tc, dict, records[i], records[i], boolean_true);
	}

	cpp_wrapper_all_records(tc, dict, records, records_length, boolean_true);
}

/**
@brief	Aggregate test to test an all records cursor query on all dictionary implementations
		where keys are not inserted in increasing order and include a wide range.
*/
void
test_cpp_wrapper_all_records_random_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records_random(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_all_records_random(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_all_records_random(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_all_records_random(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_all_records_random(tc, dict);
	delete dict;
}

/**
@brief	Aggregate test to test open/close functionality on all dictionary implementations.
*/
void
test_cpp_wrapper_open_close_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	cpp_wrapper_open_close(tc, dict, 66, 12);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	cpp_wrapper_open_close(tc, dict, 45, 14);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	cpp_wrapper_open_close(tc, dict, 3, 15);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	cpp_wrapper_open_close(tc, dict, 5, 12);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	cpp_wrapper_open_close(tc, dict, 1, 13);
	delete dict;

/*	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7); */
/*	cpp_wrapper_open_close(tc, dict, 2, 22); */
/*	delete dict; */
}

/**
@brief	This function tests deleting a closed instance of the master table.
*/
void
test_master_table_open_close(
	planck_unit_test_t *tc
) {
	MasterTable *master_table = new MasterTable();

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);
	master_table_close(tc, master_table);
	delete master_table;
}

/**
@brief	This function tests deleting an open instance of the master table.
*/
void
test_master_table_delete(
	planck_unit_test_t *tc
) {
	MasterTable *master_table = new MasterTable();

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);
	delete master_table;
}

/**
@brief	This function tests creating and deleting a dictionary using the master table.
*/
void
test_master_table_dictionary_create_delete(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	MasterTable *master_table = new MasterTable();

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);
	master_table_dictionary_add(tc, master_table, dictionary, key_type_numeric_signed, sizeof(int), sizeof(int), dictionary_size, dictionary_type);

	ion_dictionary_id_t id = dictionary->dict.instance->id;

	delete dictionary;

	master_table_delete_from_master_table(tc, master_table, id);

	delete master_table;
}

/**
@brief Tests creating and deleting a dictionary using the master table on all
		dictionary implementations.
*/
void
test_master_table_dictionary_create_delete_all_1(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dictionary;

	dictionary = new BppTree<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 0, dictionary_type_bpp_tree_t);

	dictionary = new FlatFile<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 30, dictionary_type_flat_file_t);

	dictionary = new OpenAddressHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 50, dictionary_type_open_address_hash_t);

	dictionary = new OpenAddressFileHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 50, dictionary_type_open_address_file_hash_t);

	dictionary = new SkipList<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 7, dictionary_type_skip_list_t);

	dictionary = new LinearHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 7, dictionary_type_linear_hash_t);
}

/**
@brief	This function tests creating and deleting a dictionary using the master table.
*/
void
test_master_table_dictionary_create_delete(
	planck_unit_test_t		*tc,
	ion_dictionary_size_t	dictionary_size,
	ion_dictionary_type_t	dictionary_type
) {
	MasterTable *master_table = new MasterTable();

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);

	Dictionary<int, int> *dictionary;

	int type = 0;

	dictionary = master_table->initializeDictionary(key_type_numeric_signed, type, type, sizeof(int), sizeof(int), dictionary_size, dictionary_type);
	master_table_create_dictionary(tc, master_table, dictionary, 1, key_type_numeric_signed, sizeof(int), sizeof(int), dictionary_size, dictionary_type);
	master_table_delete_dictionary(tc, master_table, dictionary);

	delete master_table;
}

/**
@brief Tests creating and deleting a dictionary using the master table on all
		dictionary implementations created using the master table.
*/
void
test_master_table_dictionary_create_delete_all_2(
	planck_unit_test_t *tc
) {
	test_master_table_dictionary_create_delete(tc, 0, dictionary_type_bpp_tree_t);

	test_master_table_dictionary_create_delete(tc, 30, dictionary_type_flat_file_t);

	test_master_table_dictionary_create_delete(tc, 50, dictionary_type_open_address_hash_t);

	test_master_table_dictionary_create_delete(tc, 50, dictionary_type_open_address_file_hash_t);

	test_master_table_dictionary_create_delete(tc, 7, dictionary_type_skip_list_t);

	test_master_table_dictionary_create_delete(tc, 7, dictionary_type_linear_hash_t);
}

/**
@brief	This function tests opening and closing a dictionary using the master table.
*/
void
test_master_table_dictionary_open_close(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	MasterTable *master_table = new MasterTable();

	ion_dictionary_id_t id;

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);
	master_table_dictionary_add(tc, master_table, dictionary, key_type_numeric_signed, sizeof(int), sizeof(int), dictionary_size, dictionary_type);

	id = dictionary->dict.instance->id;

	master_table_close_dictionary(tc, master_table, dictionary);
	master_table_open_dictionary(tc, master_table, dictionary, id);

	delete dictionary;

	master_table_delete_from_master_table(tc, master_table, id);

	delete master_table;
}

/**
@brief Tests opening and closing a dictionary using the master table on all
		dictionary implementations.
*/
void
test_master_table_dictionary_open_close_all_1(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dictionary;

	dictionary = new BppTree<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 0, dictionary_type_bpp_tree_t);

	dictionary = new FlatFile<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 30, dictionary_type_flat_file_t);

	dictionary = new OpenAddressHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 50, dictionary_type_open_address_hash_t);

	dictionary = new OpenAddressFileHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 50, dictionary_type_open_address_file_hash_t);

	dictionary = new SkipList<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 7, dictionary_type_skip_list_t);

	/* Uncomment when LinearHash dictionary open memory issue fixed. */
/*	dictionary = new LinearHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 7); */
/*	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status); */
/*	test_master_table_dictionary_open_close(tc, dictionary, 7, dictionary_type_linear_hash_t); */
}

/**
@brief	This function tests opening and closing a dictionary using the master table.
*/
void
test_master_table_dictionary_open_close(
	planck_unit_test_t		*tc,
	ion_dictionary_size_t	dictionary_size,
	ion_dictionary_type_t	dictionary_type
) {
	MasterTable *master_table = new MasterTable();

	ion_dictionary_id_t id;

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);

	Dictionary<int, int> *dictionary;

	int type = 0;

	dictionary	= master_table->initializeDictionary(key_type_numeric_signed, type, type, sizeof(int), sizeof(int), dictionary_size, dictionary_type);
	master_table_create_dictionary(tc, master_table, dictionary, 1, key_type_numeric_signed, sizeof(int), sizeof(int), dictionary_size, dictionary_type);

	id			= dictionary->dict.instance->id;

	master_table_close_dictionary(tc, master_table, dictionary);
	master_table_open_dictionary(tc, master_table, dictionary, id);

	master_table_delete_dictionary(tc, master_table, dictionary);

	delete master_table;
}

/**
@brief Tests opening and closing a dictionary using the master table on all
		dictionary implementations created using the master table.
*/
void
test_master_table_dictionary_open_close_all_2(
	planck_unit_test_t *tc
) {
	test_master_table_dictionary_open_close(tc, 0, dictionary_type_bpp_tree_t);

	test_master_table_dictionary_open_close(tc, 30, dictionary_type_flat_file_t);

	test_master_table_dictionary_open_close(tc, 50, dictionary_type_open_address_hash_t);

	test_master_table_dictionary_open_close(tc, 50, dictionary_type_open_address_file_hash_t);

	test_master_table_dictionary_open_close(tc, 7, dictionary_type_skip_list_t);

	/* Uncomment when LinearHash dictionary open memory issue fixed. */
/*	test_master_table_dictionary_open_close(tc, 7, dictionary_type_linear_hash_t); */
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper_getsuite_1(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_setup_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_insert_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_insert_multiple_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_in_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_lots_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_exist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_populated_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_populated_multiple_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_all_all);

	return suite;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper_getsuite_2(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_nonexist_several_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_single_several_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_all_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_empty_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_nonexist_in_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_exist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_exist_in_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_all_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_then_insert_all);

	return suite;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper_getsuite_3(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_equality_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_equality_nonexist_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_equality_nonexist_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_multiple_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_nonexist_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_nonexist_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_exist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_all_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_all_records_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_all_records_nonexist_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_all_records_populated_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_all_records_random_all);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_open_close_all);

	return suite;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper_getsuite_4(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_open_close);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_delete);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_dictionary_create_delete_all_1);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_dictionary_create_delete_all_2);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_dictionary_open_close_all_1);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_dictionary_open_close_all_2);

	return suite;
}

/**
@brief	  Runs all C++ Related related tests and outputs the result.
*/
void
runalltests_cpp_wrapper(
) {
	fdeleteall();

	planck_unit_suite_t *suite1 = cpp_wrapper_getsuite_1();

	planck_unit_run_suite(suite1);
	planck_unit_destroy_suite(suite1);

	planck_unit_suite_t *suite2 = cpp_wrapper_getsuite_2();

	planck_unit_run_suite(suite2);
	planck_unit_destroy_suite(suite2);

	planck_unit_suite_t *suite3 = cpp_wrapper_getsuite_3();

	planck_unit_run_suite(suite3);
	planck_unit_destroy_suite(suite3);

	planck_unit_suite_t *suite4 = cpp_wrapper_getsuite_4();

	planck_unit_run_suite(suite4);
	planck_unit_destroy_suite(suite4);
}
