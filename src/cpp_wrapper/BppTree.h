/******************************************************************************/
/**
@file		BppTree.h
@author		Dana Klamut
@brief		C++ implementation of a B+ tree dictionary.
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

#if !defined(PROJECT_BPPTREE_H)
#define PROJECT_BPPTREE_H

#include "Dictionary.h"
#include "../key_value/kv_system.h"
#include "../dictionary/bpp_tree/bpp_tree_handler.h"

template<typename K, typename V>
class BppTree:public Dictionary<K, V> {
public:
/**
@brief		Registers a specific B+ tree dictionary instance.

@details	Registers functions for dictionary.
@param		id
				A unique identifier important for use of the dictionary through
				the master table. If the dictionary is being created without
				the master table, this identifier can be 0.
@param		key_type
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param		value_size
				The size of the values to be stored in the dictionary.
*/
BppTree(
	ion_dictionary_id_t id,
	ion_key_type_t		key_type,
	int					key_size,
	int					value_size
) {
	bpptree_init(&this->handler);

	this->initializeDictionary(id, key_type, key_size, value_size, 0);
}
};

#endif /* PROJECT_BPPTREE_H */
