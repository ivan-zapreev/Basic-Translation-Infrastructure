/* 
 * File:   MGram.hpp
 * Author: Dr. Ivan S. Zapreev
 *
 * Visit my Linked-in profile:
 *      <https://nl.linkedin.com/in/zapreevis>
 * Visit my GitHub:
 *      <https://github.com/ivan-zapreev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.#
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created on September 7, 2015, 9:42 PM
 */

#include "CompMGramId.hpp"

#include <inttypes.h>   // std::uint32_t

#include "Globals.hpp"
#include "Logger.hpp"
#include "Exceptions.hpp"

#include "TextPieceReader.hpp"
#include "AWordIndex.hpp"
#include "HashingUtils.hpp"
#include "MathUtils.hpp"

using namespace uva::utils::math::log2;
using namespace uva::utils::math::bits;
using namespace uva::smt::hashing;
using namespace uva::smt::logging;
using namespace uva::smt::file;
using namespace uva::smt::tries::dictionary;

using namespace std;

namespace uva {
    namespace smt {
        namespace tries {
            namespace mgrams {
                namespace Comp_M_Gram_Id {

                    /**
                     * This method allows to get the number of bits needed to store this word id
                     * @param wordId the word id to analyze
                     * @return the number of bits needed to store this word id
                     */
                    static inline uint8_t get_number_of_bits(const uint32_t wordId) {
                        return log2_32(wordId) + 1;
                    };

                    /**
                     * Stores the multipliers up to and including level 6
                     */
                    const static uint32_t gram_id_type_mult[] = {1, 32, 32 * 32, 32 * 32 * 32, 32 * 32 * 32 * 32, 32 * 32 * 32 * 32 * 32};

                    /**
                     * This method is needed to compute the id type identifier.
                     * Can compute the id type for M-grams until (including) M = 6
                     * The type is computed as in a 32-based numeric system, e.g. for M==5:
                     *          (len_bits[0]-1)*32^0 + (len_bits[1]-1)*32^1 +
                     *          (len_bits[2]-1)*32^2 + (len_bits[3]-1)*32^3 +
                     *          (len_bits[4]-1)*32^4
                     * @param the number of word ids
                     * @param len_bits the bits needed per word id
                     * @param id_type [out] the resulting id type the initial value is expected to be 0
                     */
                    template<TModelLevel M_GRAM_LEVEL>
                    inline void gram_id_bit_len_2_type(uint8_t len_bits[M_GRAM_LEVEL], uint32_t & id_type) {
                        //Do the sanity check for against overflows
                        if (DO_SANITY_CHECKS && (M_GRAM_LEVEL > M_GRAM_LEVEL_6)) {
                            stringstream msg;
                            msg << "get_gram_id_type: Unsupported m-gram level: "
                                    << SSTR(M_GRAM_LEVEL) << ", must be within ["
                                    << SSTR(M_GRAM_LEVEL_2) << ", "
                                    << SSTR(M_GRAM_LEVEL_6) << "], insufficient multipliers!";
                            throw Exception(msg.str());
                        }

                        LOG_DEBUG3 << "Computing the " << SSTR(M_GRAM_LEVEL) << "-gram id type" << END_LOG;

                        //Compute the M-gram id type. Here we use the pre-computed multipliers
                        for (size_t idx = 0; idx < M_GRAM_LEVEL; ++idx) {
                            LOG_DEBUG3 << ((uint32_t) len_bits[idx] - 1) << " * " << gram_id_type_mult[idx] << " =  "
                                    << ((uint32_t) len_bits[idx] - 1) * gram_id_type_mult[idx] << END_LOG;
                            id_type += ((uint32_t) len_bits[idx] - 1) * gram_id_type_mult[idx];
                        }
                        LOG_DEBUG3 << "Resulting id_type = " << SSTR(id_type) << END_LOG;
                    };

                    /**
                     * Allows to compute the bit length for the id of the given type
                     * @param M_GRAM_LEVEL the M-Gram level M
                     * @param id_type the type id
                     * @param id_len_bits [out] the total bit length to store the id of this type
                     */
                    template<TModelLevel M_GRAM_LEVEL>
                    inline void gram_id_type_2_bit_len(uint32_t id_type, uint8_t & id_len_bits) {
                        //2. Compute the M-gram id length from the M-gram id type.
                        //   Here we use the pre-computed multipliers we add the
                        //   final bits at the end of the function.
                        uint8_t coeff = 0;
                        for (int idx = (M_GRAM_LEVEL - 1); idx >= 0; --idx) {
                            coeff = (uint8_t) (id_type / gram_id_type_mult[idx]);
                            LOG_DEBUG3 << SSTR(id_type) << " / " << SSTR(gram_id_type_mult[idx]) << " =  " << SSTR((uint32_t) coeff) << END_LOG;
                            id_type = (uint8_t) (id_type % gram_id_type_mult[idx]);
                            id_len_bits += coeff;
                        }
                        //Note that in the loop above we have "coeff = len_bits[idx] - 1"
                        //Therefore, here we add the number of tokens to account for this -1's
                        id_len_bits += (uint8_t) M_GRAM_LEVEL;
                    }

                    /**
                     * Allows to extract the M-gram id length from the given M-gram level M and id
                     * This implementation should work up to 6-grams! If we use it for
                     * 7-grams then the internal computations will overflow!
                     * @param ID_TYPE_LEN_BITS the maximum number of bites needed to store the id type in bits
                     * @param M_GRAM_LEVEL the number of tokens in the M-gram
                     * @param m_gram_id the given M-gram id
                     * @param len_bytes [out] the M-gram id length in bytes
                     */
                    template<uint8_t ID_TYPE_LEN_BITS, TModelLevel M_GRAM_LEVEL >
                    void get_gram_id_len(const uint8_t * m_gram_id, uint8_t & len_bytes) {
                        //If needed, do the sanity checks
                        if (DO_SANITY_CHECKS) {
                            if (m_gram_id == NULL) {
                                throw Exception("get_m_gram_id_length: A NULL pointer M-gram id!");
                            }
                            if (M_GRAM_LEVEL > M_GRAM_LEVEL_6) {
                                stringstream msg;
                                msg << "get_gram_id_len: Unsupported m-gram level: "
                                        << SSTR(M_GRAM_LEVEL) << ", must be within ["
                                        << SSTR(M_GRAM_LEVEL_2) << ", "
                                        << SSTR(M_GRAM_LEVEL_6) << "], otherwise overflows!";
                                throw Exception(msg.str());
                            }
                        }

                        //1. ToDo: Extract the id_type from the M-gram
                        uint32_t id_type = 0;
                        copy_begin_bits_to_end<ID_TYPE_LEN_BITS>(m_gram_id, id_type);

                        //2. Compute the M-gram id length from the M-gram id type.
                        //   Here we use the pre-computed multipliers we add the
                        //   final bits at the end of the function.

                        //Declare and initialize the id length, the initial values is
                        //what we need to store the type. Note that, the maximum number
                        //of needed bits for an id for a 5-gram is 25+32+32+32+32+32 = 185
                        //bits. For a 6-gram we get 30+32+32+32+32+32+32 = 222 bits,
                        //for a 7-gram we get 35+32+32+32+32+32+32+32 = 259 bits!
                        //This is when we get an overflow here if we use uint8_t to
                        //store the length in bits will overflow. Therefore the sanity check.
                        uint8_t id_len_bits = ID_TYPE_LEN_BITS;
                        gram_id_type_2_bit_len<M_GRAM_LEVEL>(id_type, id_len_bits);

                        LOG_DEBUG3 << "ID_TYPE_LEN_BITS: " << SSTR((uint32_t) ID_TYPE_LEN_BITS)
                                << ", Total id_len_bits: " << SSTR((uint32_t) id_len_bits) << END_LOG;

                        len_bytes = NUM_BITS_TO_STORE_BYTES(id_len_bits);
                    };

                    /**
                     * This method is needed to compute the M-gram id.
                     * 
                     * This implementation should work up to 6-grams! If we use it for
                     * 7-grams then the internal computations will overflow!
                     * 
                     * Let us give an example of a 2-gram id for a given 2-gram:
                     * 
                     * 1) The 2 word_ids are to be converted to the 2-gram id:
                     * There are 32 bytes in one word id and 32 bytes in
                     * another word id, In total we have 32^2 possible 2-gram id
                     * lengths, if we only use meaningful bits of the word id for
                     * instance:
                     *       01-01 both really need just one bite
                     *       01-02 the first needs one and another two
                     *       02-01 the first needs two and another one
                     *       ...
                     *       32-32 both need 32 bits
                     * 
                     * 2) These 32^2 = 1,024 combinations uniquely identify the
                     * type of stored id. So this can be an uid of the gram id type.
                     * To store such a uid we need log2(1,024)= 10 bits.
                     * 
                     * 3) We create the 2-gram id as a byte array of 10 bits - type
                     * + the meaningful bits from wordId2 and wordId1. We start from
                     * the end (reverse the word order) as this can potentially
                     * increase speed of the comparison operation.
                     * 
                     * 4) The total length of the 2-gram id in bytes is the number
                     * of bits needed to store the key type and meaningful word id
                     * bits, rounded up to the full bytes.
                     * 
                     * @param ID_TYPE_LEN_BITS the maximum number of bites needed to store the id type in bits
                     * @param M_GRAM_LEVEL the number of tokens in the M-gram
                     * @param word_ids the pointer to the array of word ids
                     * @param m_p_gram_id the pointer to the data storage to be initialized.
                     * If (*m_p_p_gram_id == NULL) new memory will be allocated for the id, otherwise the
                     * pointer will be used to store the id. It is then assumed that there is enough memory
                     * allocated to store the id. For an M-gram it is (4*M) bytes that is needed to store
                     * the longed M-gram id.
                     */
                    template<uint8_t ID_TYPE_LEN_BITS, TModelLevel M_GRAM_LEVEL>
                    static inline void create_gram_id(const TShortId * word_ids,
                            T_Gram_Id_Storage_Ptr & m_p_gram_id) {
                        //Declare and initialize the id length, the initial values is
                        //what we need to store the type. Note that, the maximum number
                        //of needed bits for an id for a 5-gram is 25+32+32+32+32+32 = 185
                        //bits. For a 6-gram we get 30+32+32+32+32+32+32 = 222 bits,
                        //for a 7-gram we get 35+32+32+32+32+32+32+32 = 259 bits!
                        //This is when we get an overflow here if we use uint8_t to
                        //store the length in bits will overflow. Therefore the sanity check.
                        uint8_t id_len_bits = ID_TYPE_LEN_BITS;

                        LOG_DEBUG3 << "Creating the " << SSTR(M_GRAM_LEVEL) << "-gram id with "
                                << "id type length: " << SSTR((uint32_t) ID_TYPE_LEN_BITS) << END_LOG;

                        //Obtain the word ids and their lengths in bits and
                        //the total length in bits needed to store the key
                        uint8_t len_bits[M_GRAM_LEVEL];
                        for (size_t idx = 0; idx < M_GRAM_LEVEL; ++idx) {
                            //Get the number of bits needed to store this id
                            len_bits[idx] = get_number_of_bits(word_ids[idx]);

                            LOG_DEBUG3 << "Word id: " << SSTR(word_ids[idx])
                                    << " len. in bits: " << SSTR((uint32_t) len_bits[idx]) << END_LOG;

                            //Compute the total gram id length in bits
                            id_len_bits += len_bits[idx];
                        }
                        LOG_DEBUG3 << "Total len. in bits: " << SSTR((uint32_t) id_len_bits) << END_LOG;

                        //Determine the size of id in bytes, divide with rounding up
                        const uint8_t id_len_bytes = NUM_BITS_TO_STORE_BYTES(id_len_bits);

                        LOG_DEBUG3 << "Total len. in bytes: " << SSTR((uint32_t) id_len_bytes) << END_LOG;

                        //Allocate the id memory if there was nothing pre-allocated yet
                        if (m_p_gram_id == NULL) {
                            //Allocate memory
                            m_p_gram_id = new uint8_t[id_len_bytes];
                            LOG_DEBUG3 << "Created a Compressed_M_Gram_Id: " << SSTR((void *) m_p_gram_id) << END_LOG;
                        }
                        //Clean the memory
                        memset(m_p_gram_id, 0, id_len_bytes);

                        //Determine the type id value from the bit lengths of the words
                        uint32_t id_type_value = 0;
                        gram_id_bit_len_2_type<M_GRAM_LEVEL>(len_bits, id_type_value);
                        LOG_DEBUG3 << "ID_TYPE_LEN_BITS: " << (uint32_t) ID_TYPE_LEN_BITS << ", id_type_value: " << id_type_value << END_LOG;

                        //Append the id type to the M-gram id
                        uint8_t to_bit_pos = 0;
                        copy_end_bits_to_pos(id_type_value, ID_TYPE_LEN_BITS, m_p_gram_id, to_bit_pos);
                        to_bit_pos += ID_TYPE_LEN_BITS;

                        //Append the word id meaningful bits to the id in reverse order
                        for (int idx = (M_GRAM_LEVEL - 1); idx >= 0; --idx) {
                            copy_end_bits_to_pos(word_ids[idx], len_bits[idx], m_p_gram_id, to_bit_pos);
                            to_bit_pos += len_bits[idx];
                        }

                        LOG_DEBUG3 << "Finished making the " << SSTR(M_GRAM_LEVEL) << "-gram id with "
                                << "id type length: " << SSTR((uint32_t) ID_TYPE_LEN_BITS)
                                << ", bits: " << bytes_to_bit_string(m_p_gram_id, id_len_bytes) << END_LOG;
                    };

                    static inline void create_2_gram_id(const TShortId * word_ids,
                            T_Gram_Id_Storage_Ptr & m_p_gram_id) {
                        create_gram_id < M_GRAM_ID_TYPE_LEN_BITS[M_GRAM_LEVEL_2], M_GRAM_LEVEL_2 >
                                (word_ids, m_p_gram_id);
                    }

                    static inline void create_3_gram_id(const TShortId * word_ids,
                            T_Gram_Id_Storage_Ptr & m_p_gram_id) {
                        create_gram_id < M_GRAM_ID_TYPE_LEN_BITS[M_GRAM_LEVEL_3], M_GRAM_LEVEL_3 >
                                (word_ids, m_p_gram_id);
                    }

                    static inline void create_4_gram_id(const TShortId * word_ids,
                            T_Gram_Id_Storage_Ptr & m_p_gram_id) {
                        create_gram_id < M_GRAM_ID_TYPE_LEN_BITS[M_GRAM_LEVEL_4], M_GRAM_LEVEL_4 >
                                (word_ids, m_p_gram_id);
                    }

                    static inline void create_5_gram_id(const TShortId * word_ids,
                            T_Gram_Id_Storage_Ptr & m_p_gram_id) {
                        create_gram_id < M_GRAM_ID_TYPE_LEN_BITS[M_GRAM_LEVEL_5], M_GRAM_LEVEL_5 >
                                (word_ids, m_p_gram_id);
                    }

                    /**
                     * Define the function pointer to a create x-gram id function for some X-gram level x
                     */
                    typedef void(*create_x_gram_id)(const TShortId * word_ids, T_Gram_Id_Storage_Ptr & m_p_gram_id);

                    //This is an array of functions for creating m-grams per specific m-gram level m
                    const static create_x_gram_id create_x_gram_funcs[] = {NULL, NULL,
                        create_2_gram_id, create_3_gram_id, create_4_gram_id, create_5_gram_id};

                    void create_m_gram_id(const TShortId * word_ids,
                            const uint8_t begin_idx, const uint8_t num_word_ids,
                            T_Gram_Id_Storage_Ptr & m_p_gram_id) {

                        if (DO_SANITY_CHECKS &&
                                ((num_word_ids < M_GRAM_LEVEL_2) || (num_word_ids > M_GRAM_LEVEL_5))) {
                            stringstream msg;
                            msg << "create_m_gram_id: Unsupported m-gram level: "
                                    << SSTR(num_word_ids) << ", must be within ["
                                    << SSTR(M_GRAM_LEVEL_2) << ", "
                                    << SSTR(M_GRAM_LEVEL_5) << "]";
                            throw Exception(msg.str());
                        }

                        //Call the appropriate function, use array instead of switch, should be faster.
                        create_x_gram_funcs[num_word_ids](&word_ids[begin_idx], m_p_gram_id);
                    }
#define IS_LESS -1
#define IS_EQUAL 0
#define IS_LARGER +1

                    template<TModelLevel M_GRAM_LEVEL>
                    int compare(const T_Gram_Id_Storage_Ptr & m_p_gram_id_one, const T_Gram_Id_Storage_Ptr & m_p_gram_id_two) {

                        //Get the id len in bits
                        constexpr uint8_t ID_TYPE_LEN_BITS = M_GRAM_ID_TYPE_LEN_BITS[M_GRAM_LEVEL];

                        //Get the M-gram type ids
                        TShortId type_one = 0;
                        copy_begin_bits_to_end < ID_TYPE_LEN_BITS >(m_p_gram_id_one, type_one);
                        TShortId type_two = 0;
                        copy_begin_bits_to_end < ID_TYPE_LEN_BITS >(m_p_gram_id_two, type_two);

                        LOG_DEBUG3 << "M_GRAM_LEVEL: " << (uint32_t) M_GRAM_LEVEL << ", type_one: " << type_one << ", type_two: " << type_two << END_LOG;

                        if (type_one < type_two) {
                            //The first id type is smaller
                            LOG_DEBUG3 << (void*) m_p_gram_id_one << " < " << (void*) m_p_gram_id_two << END_LOG;
                            return IS_LESS;
                        } else {
                            if (type_one > type_two) {
                                //The second id type is smaller
                                LOG_DEBUG3 << (void*) m_p_gram_id_one << " > " << (void*) m_p_gram_id_two << END_LOG;
                                return IS_LARGER;
                            } else {
                                //The id types are the same! Compare the ids themselves
                                LOG_DEBUG3 << (void*) m_p_gram_id_one << " =(type)= " << (void*) m_p_gram_id_two << END_LOG;

                                //Get one of the lengths, as they both are the same
                                uint8_t id_len_bytes = 0;
                                get_gram_id_len < ID_TYPE_LEN_BITS, M_GRAM_LEVEL > (m_p_gram_id_one, id_len_bytes);

                                //Start comparing the ids but not from the fist bytes as
                                //this is where the id type information is stored, start
                                //with the first byte that contains key information
                                const uint8_t num_bytes_to_skip = NUM_FULL_BYTES(ID_TYPE_LEN_BITS);

                                LOG_DEBUG3 << "ID_TYPE_LEN_BITS: " << SSTR((uint32_t) ID_TYPE_LEN_BITS)
                                        << ", start comparing id key from idx: " << SSTR((uint32_t) num_bytes_to_skip)
                                        << ", Total id_len_bytes: " << SSTR((uint32_t) id_len_bytes) << END_LOG;

                                LOG_DEBUG3 << (void*) m_p_gram_id_one << " = " << bytes_to_bit_string(m_p_gram_id_one, id_len_bytes) << END_LOG;
                                LOG_DEBUG3 << (void*) m_p_gram_id_two << " = " << bytes_to_bit_string(m_p_gram_id_two, id_len_bytes) << END_LOG;

                                //Compare with the fast system function
                                return memcmp(m_p_gram_id_one + num_bytes_to_skip, m_p_gram_id_two + num_bytes_to_skip, (id_len_bytes - num_bytes_to_skip));
                            }
                        }
                    };

                    /***********************************************************************************************************************/

                    /**
                     * Define the function pointer to compare two X-grams of the given level X
                     */
                    typedef bool(* is_compare_grams_id_func)(const T_Gram_Id_Storage_Ptr &, const T_Gram_Id_Storage_Ptr &);

                    /******************************************/

                    static inline bool is_equal_2_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare< M_GRAM_LEVEL_2>(one, two) == 0);
                    }

                    static inline bool is_equal_3_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_3>(one, two) == 0);
                    }

                    static inline bool is_equal_4_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_4>(one, two) == 0);
                    }

                    static inline bool is_equal_5_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_5>(one, two) == 0);
                    }

                    //This is an array of functions for comparing x-grams of level x
                    const static is_compare_grams_id_func is_equal_x_grams_id_funcs[] = {NULL, NULL,
                        is_equal_2_grams_id, is_equal_3_grams_id,
                        is_equal_4_grams_id, is_equal_5_grams_id};

                    bool is_equal_m_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two, const TModelLevel level) {
                        return is_equal_x_grams_id_funcs[level](one, two);
                    }

                    /******************************************/

                    static inline bool is_less_2_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_2>(one, two) < 0);
                    }

                    static inline bool is_less_3_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_3>(one, two) < 0);
                    }

                    static inline bool is_less_4_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_4>(one, two) < 0);
                    }

                    static inline bool is_less_5_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_5>(one, two) < 0);
                    }

                    //This is an array of functions for comparing x-grams of level x
                    const static is_compare_grams_id_func is_less_x_grams_id_funcs[] = {NULL, NULL,
                        is_less_2_grams_id, is_less_3_grams_id,
                        is_less_4_grams_id, is_less_5_grams_id};

                    bool is_less_m_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two, const TModelLevel level) {
                        return is_less_x_grams_id_funcs[level](one, two);
                    }

                    /******************************************/

                    static inline bool is_more_2_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare< M_GRAM_LEVEL_2>(one, two) > 0);
                    }

                    static inline bool is_more_3_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_3>(one, two) > 0);
                    }

                    static inline bool is_more_4_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_4>(one, two) > 0);
                    }

                    static inline bool is_more_5_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two) {
                        return (compare<M_GRAM_LEVEL_5>(one, two) > 0);
                    }

                    //This is an array of functions for comparing x-grams of level x
                    const static is_compare_grams_id_func is_more_x_grams_id_funcs[] = {NULL, NULL,
                        is_more_2_grams_id, is_more_3_grams_id,
                        is_more_4_grams_id, is_more_5_grams_id};

                    bool is_more_m_grams_id(const T_Gram_Id_Storage_Ptr & one, const T_Gram_Id_Storage_Ptr & two, const TModelLevel level) {
                        return is_more_x_grams_id_funcs[level](one, two);
                    }

                    template int compare<M_GRAM_LEVEL_2>(const T_Gram_Id_Storage_Ptr & m_p_gram_id_one, const T_Gram_Id_Storage_Ptr & m_p_gram_id_two);
                    template int compare<M_GRAM_LEVEL_3>(const T_Gram_Id_Storage_Ptr & m_p_gram_id_one, const T_Gram_Id_Storage_Ptr & m_p_gram_id_two);
                    template int compare<M_GRAM_LEVEL_4>(const T_Gram_Id_Storage_Ptr & m_p_gram_id_one, const T_Gram_Id_Storage_Ptr & m_p_gram_id_two);
                    template int compare<M_GRAM_LEVEL_5>(const T_Gram_Id_Storage_Ptr & m_p_gram_id_one, const T_Gram_Id_Storage_Ptr & m_p_gram_id_two);
                }
            }
        }
    }
}