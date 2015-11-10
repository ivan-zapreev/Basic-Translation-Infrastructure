/* 
 * File:   C2WOrderedArrayTrie.hpp
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
 * Created on August 25, 2015, 11:10 AM
 */

#ifndef C2WORDEREDARRAYTRIE_HPP
#define	C2WORDEREDARRAYTRIE_HPP

#include <string>       // std::string

#include "Globals.hpp"
#include "Logger.hpp"

#include "LayeredTrieBase.hpp"

#include "AWordIndex.hpp"
#include "ArrayUtils.hpp"

using namespace std;
using namespace uva::smt::tries::dictionary;
using namespace uva::smt::utils::array;

namespace uva {
    namespace smt {
        namespace tries {
            namespace __C2WArrayTrie {

                /**
                 * This structure stores two things the word id
                 * and the corresponding probability/back-off data.
                 * It is used to store the M-gram data for levels 1 < M < N.
                 * @param id the word id
                 * @param data the back-off and probability data
                 */
                typedef struct {
                    TShortId id;
                    T_M_Gram_Payload data;
                } TWordIdPBData;

                /**
                 * This is the less operator implementation
                 * @param one the first object to compare
                 * @param two the second object to compare
                 * @return true one.id < two.id
                 */
                inline bool operator<(const TWordIdPBData & one, const TWordIdPBData & two) {
                    return (one.id < two.id);
                }

                /**
                 * Stores the information about the context id, word id and corresponding probability
                 * This data structure is to be used for the N-Gram data, as there are no back-offs
                 * It is used to store the N-gram data for the last Trie level N.
                 * @param ctx_id the context id
                 * @param word_id the word id
                 * @param prob the probability data
                 */
                typedef struct {
                    TShortId word_id;
                    TShortId ctx_id;
                    TLogProbBackOff prob;
                } TCtxIdProbData;

                /**
                 * This is the compare operator implementation
                 * @param one the first object to compare
                 * @param two the second object to compare
                 * @return -1 if (word_id,ctx_id) < (word_id,ctx_id)
                 *          0 if (word_id,ctx_id) == (word_id,ctx_id)
                 *         +1 if (word_id,ctx_id) > (word_id,ctx_id)
                 */
                inline int8_t compare(const TCtxIdProbData & one, const TCtxIdProbData & two) {
                    if (one.word_id < two.word_id) {
                        return -1;
                    } else {
                        if (one.word_id == two.word_id) {
                            if (one.ctx_id < two.ctx_id) {
                                return -1;
                            } else {
                                if (one.ctx_id == two.ctx_id) {
                                    return 0;
                                } else {
                                    return +1;
                                }
                            }
                        } else {
                            return +1;
                        }
                    }
                };

                //An alternative check: Is a tiny biut slower
                //const TLongId key1 = TShortId_TShortId_2_TLongId(one.word_id, one.ctx_id);
                //const TLongId key2 = TShortId_TShortId_2_TLongId(two.word_id, two.ctx_id);
                //return (key1 < key2);

                inline bool operator<(const TCtxIdProbData & one, const TCtxIdProbData & two) {
                    return (compare(one, two) < 0);
                };

                inline bool operator>(const TCtxIdProbData & one, const TCtxIdProbData & two) {
                    return (compare(one, two) > 0);
                };

                inline bool operator==(const TCtxIdProbData & one, const TCtxIdProbData & two) {
                    return (compare(one, two) == 0);
                };
            }

            /**
             * This is the Context to word array memory trie implementation class.
             * 
             * WARNING: This trie assumes that the M-grams (1 <= M < N) are added
             * to the Trie in an ordered way and there are no duplicates in the
             * 1-Grams. The order is assumed to be lexicographical as in the ARPA
             * files! This is also checked if the sanity checks are on see Globals.hpp!
             * 
             * @param N the maximum number of levels in the trie.
             */
            template<TModelLevel MAX_LEVEL, typename WordIndexType>
            class C2WArrayTrie : public LayeredTrieBase<MAX_LEVEL, WordIndexType> {
            public:
                typedef LayeredTrieBase<MAX_LEVEL, WordIndexType> BASE;

                /**
                 * The basic constructor
                 * @param p_word_index the word index (dictionary) container
                 */
                explicit C2WArrayTrie(WordIndexType & p_word_index);

                /**
                 * @see GenericTrieBase
                 */
                constexpr static inline bool needs_bitmap_hash_cache() {
                    return __C2WArrayTrie::DO_BITMAP_HASH_CACHE;
                }

                /**
                 * Computes the N-Gram context using the previous context and the current word id
                 * 
                 * WARNING: Must only be called for the M-gram level 1 < M < N!
                 * @see LayeredTrieBase
                 * 
                 * @param word_id the current word id
                 * @param ctx_id [in] - the previous context id, [out] - the next context id
                 * @param level the M-gram level we are working with M
                 * @return the resulting context
                 * @throw nothing
                 */
                template<TModelLevel level>
                inline bool get_ctx_id(const TShortId word_id, TLongId & ctx_id) const {
                    //Compute the m-gram index
                    const TModelLevel mgram_idx = level - BASE::MGRAM_IDX_OFFSET;

                    if (DO_SANITY_CHECKS && ((level == MAX_LEVEL) || (mgram_idx < 0))) {
                        stringstream msg;
                        msg << "Unsupported level id: " << level;
                        throw Exception(msg.str());
                    }

                    LOG_DEBUG2 << "Searching for the next ctx_id of " << SSTR(level)
                            << "-gram with word_id: " << SSTR(word_id) << ", ctx_id: "
                            << SSTR(ctx_id) << END_LOG;

                    //First get the sub-array reference. Note that, even if it is the 2-Gram
                    //case and the previous word is unknown (ctx_id == 0) we still can use
                    //the ctx_id to get the data entry. The reason is that we allocated memory
                    //for it but being for an unknown word context it should have no data!
                    TSubArrReference & ref = m_M_gram_ctx_2_data[mgram_idx][ctx_id];

                    LOG_DEBUG2 << "Got context mapping for ctx_id: " << SSTR(ctx_id)
                            << ", with beginIdx: " << SSTR(ref.beginIdx) << ", endIdx: "
                            << SSTR(ref.endIdx) << END_LOG;

                    //Check that there is data for the given context available
                    if (ref.beginIdx != BASE::UNDEFINED_ARR_IDX) {
                        TShortId nextCtxId = BASE::UNDEFINED_ARR_IDX;
                        //The data is available search for the word index in the array
                        //WARNING: The linear search here is much slower!!!
                        if (my_bsearch_id<TWordIdPBEntry>(m_M_gram_data[mgram_idx], ref.beginIdx, ref.endIdx, word_id, nextCtxId)) {
                            LOG_DEBUG2 << "The next ctx_id for word_id: " << SSTR(word_id) << ", ctx_id: "
                                    << SSTR(ctx_id) << " is nextCtxId: " << SSTR(nextCtxId) << END_LOG;
                            ctx_id = nextCtxId;
                            return true;
                        } else {
                            LOG_DEBUG2 << "Unable to find M-gram ctx_id for level: "
                                    << SSTR(level) << ", prev ctx_id: " << SSTR(ctx_id)
                                    << ", word_id: " << SSTR(word_id) << ", is not in the available range: ["
                                    << SSTR(m_M_gram_data[mgram_idx][ref.beginIdx].id) << " ... "
                                    << SSTR(m_M_gram_data[mgram_idx][ref.endIdx].id) << "]" << END_LOG;
                            return false;
                        }
                    } else {
                        LOG_DEBUG2 << "Unable to find M-gram context id for level: "
                                << SSTR(level) << ", prev ctx_id: " << SSTR(ctx_id)
                                << ", nothing present in that context!" << END_LOG;
                        return false;
                    }
                }

                /**
                 * Allows to log the information about the instantiated trie type
                 */
                inline void log_trie_type_usage_info() const {
                    LOG_USAGE << "Using the <" << __FILE__ << "> model." << END_LOG;
                }

                /**
                 * This method can be used to provide the N-gram count information
                 * That should allow for pre-allocation of the memory
                 * For more details @see LayeredTrieBase
                 */
                virtual void pre_allocate(const size_t counts[MAX_LEVEL]);

                /**
                 * This method allows to check if post processing should be called after
                 * all the X level grams are read. This method is virtual.
                 * For more details @see WordIndexTrieBase
                 */
                template<TModelLevel level>
                bool is_post_grams() const {
                    //Check the base class and we need to do post actions
                    //for the N-grams. The N-grams level data has to be
                    //sorted see post_N_Grams method implementation below.
                    return (level > M_GRAM_LEVEL_1) || BASE::template is_post_grams<level>();
                };

                /**
                 * This method should be called after all the X level grams are read.
                 * For more details @see WordIndexTrieBase
                 */
                template<TModelLevel CURR_LEVEL>
                inline void post_grams() {
                    //Call the base class method first
                    if (BASE::template is_post_grams<CURR_LEVEL>()) {
                        BASE::template post_grams<CURR_LEVEL>();
                    }

                    //Do the post actions here
                    if (CURR_LEVEL == MAX_LEVEL) {
                        post_n_grams();
                    } else {
                        if (CURR_LEVEL > M_GRAM_LEVEL_1) {
                            post_m_grams<CURR_LEVEL>();
                        }
                    }
                };

                /**
                 * Allows to retrieve the payload for the One gram with the given Id.
                 * @see LayeredTrieBase
                 */
                inline void get_1_gram_payload(const TShortId word_id, T_M_Gram_Payload &payload) const {
                    //The data is always present.
                    payload = m_1_gram_data[word_id];
                };

                /**
                 * Allows to retrieve the data storage structure for the M gram
                 * with the given M-gram level Id. M-gram context and last word Id.
                 * If the storage structure does not exist, return a new one.
                 * For more details @see LayeredTrieBase
                 */
                template<TModelLevel CURR_LEVEL>
                inline void add_m_gram_payload(const TShortId word_id, const TLongId ctx_id, const T_M_Gram_Payload & payload) {
                    if (CURR_LEVEL == M_GRAM_LEVEL_1) {
                        //Store the payload
                        m_1_gram_data[word_id] = payload;
                    } else {
                        if (CURR_LEVEL == MAX_LEVEL) {
                            //Get the new n-gram index
                            const TShortId n_gram_idx = m_M_N_gram_next_ctx_id[BASE::N_GRAM_IDX_IN_M_N_ARR]++;

                            //Store the context and word ids
                            m_N_gram_data[n_gram_idx].ctx_id = ctx_id;
                            m_N_gram_data[n_gram_idx].word_id = word_id;

                            //Store the payload
                            m_N_gram_data[n_gram_idx].prob = payload.prob;
                        } else {
                            //Compute the m-gram index
                            const TModelLevel m_gram_idx = CURR_LEVEL - BASE::MGRAM_IDX_OFFSET;

                            //First get the sub-array reference. 
                            TSubArrReference & ref = m_M_gram_ctx_2_data[m_gram_idx][ctx_id];

                            //Get the new index and increment - this will be the new end index
                            ref.endIdx = m_M_N_gram_next_ctx_id[m_gram_idx]++;

                            //Check if there are yet no elements for this context
                            if (ref.beginIdx == BASE::UNDEFINED_ARR_IDX) {
                                //There was no elements put into this context, the begin index is then equal to the end index
                                ref.beginIdx = ref.endIdx;
                            }

                            //Store the word id
                            m_M_gram_data[m_gram_idx][ref.endIdx].id = word_id;

                            //Store the payload
                            m_M_gram_data[m_gram_idx][ref.endIdx].data = payload;
                        }
                    }
                }

                /**
                 * Allows to retrieve the payload for the M-gram defined by the end word_id and ctx_id.
                 * For more details @see LayeredTrieBase
                 */
                template<TModelLevel CURR_LEVEL>
                inline GPR_Enum get_m_gram_payload(const TShortId word_id, TLongId ctx_id,
                        T_M_Gram_Payload &payload) const {
                    //Compute the m-gram index
                    constexpr TModelLevel LEVEL_IDX = CURR_LEVEL - BASE::MGRAM_IDX_OFFSET;

                    LOG_DEBUG2 << "Getting " << SSTR(CURR_LEVEL) << "-gram with word_id: "
                            << SSTR(word_id) << ", ctx_id: " << SSTR(ctx_id) << END_LOG;

                    //Get the context id, note we use short ids here!
                    if (get_ctx_id<CURR_LEVEL>(word_id, ctx_id)) {
                        //Return the data
                        payload = m_M_gram_data[LEVEL_IDX][ctx_id].data;
                        return GPR_Enum::PAYLOAD_GPR;
                    } else {
                        //The data could not be found
                        return GPR_Enum::FAILED_GPR;
                    }
                }

                /**
                 * Allows to retrieve the payload for the N gram defined by the end word_id and ctx_id.
                 * For more details @see LayeredTrieBase
                 */
                inline GPR_Enum get_n_gram_payload(const TShortId word_id, TLongId ctx_id,
                        T_M_Gram_Payload &payload) const {
                    LOG_DEBUG2 << "Getting " << SSTR(MAX_LEVEL) << "-gram with word_id: "
                            << SSTR(word_id) << ", ctx_id: " << SSTR(ctx_id) << END_LOG;

                    //Create the search key by combining ctx and word ids, see TCtxIdProbEntryPair
                    const TLongId key = TShortId_TShortId_2_TLongId(word_id, ctx_id);
                    LOG_DEBUG4 << "Searching N-Gram: TShortId_TShortId_2_TLongId(word_id = " << SSTR(word_id)
                            << ", ctx_id = " << SSTR(ctx_id) << ") = " << SSTR(key) << END_LOG;

                    //Search for the index using binary search
                    TShortId idx = BASE::UNDEFINED_ARR_IDX;
                    if (my_bsearch_wordId_ctxId<TCtxIdProbEntry>(m_N_gram_data, BASE::FIRST_VALID_CTX_ID,
                            m_M_N_gram_num_ctx_ids[BASE::N_GRAM_IDX_IN_M_N_ARR] - 1, word_id, ctx_id, idx)) {
                        //Return the data
                        payload.prob = m_N_gram_data[idx].prob;
                        return GPR_Enum::PAYLOAD_GPR;
                    } else {
                        LOG_DEBUG1 << "Unable to find " << SSTR(MAX_LEVEL) << "-gram data for ctx_id: " << SSTR(ctx_id)
                                << ", word_id: " << SSTR(word_id) << ", key " << SSTR(key) << END_LOG;
                        return GPR_Enum::FAILED_GPR;
                    }
                }

                /**
                 * Allows to retrieve the probability and back-off weight of the unknown word
                 * @param payload the unknown word payload data
                 */
                inline void get_unk_word_payload(T_M_Gram_Payload & payload) const {
                    payload = m_1_gram_data[WordIndexType::UNKNOWN_WORD_ID];
                };

                /**
                 * The basic destructor
                 */
                virtual ~C2WArrayTrie();

            protected:

                /**
                 * This structure is needed to store begin and end index to reference pieces of an array
                 * It is used to reference sub-array ranges for the M-gram data for levels 1 < M < N.
                 * 
                 * WARNING: It is not possible to get rid of this structure as the contexts are not ordered.
                 * It is only true that the contexts will be filled one after another, but the context id 
                 * will not be increased all the time.
                 * 
                 * @param beginIdx the begin index
                 * @param endIdx the end index
                 */
                typedef struct {
                    TShortId beginIdx;
                    TShortId endIdx;
                } TSubArrReference;

                typedef __C2WArrayTrie::TWordIdPBData TWordIdPBEntry;
                typedef __C2WArrayTrie::TCtxIdProbData TCtxIdProbEntry;

                template<TModelLevel CURR_LEVEL>
                inline void post_m_grams() {
                    //Compute the m-gram index
                    constexpr TModelLevel mgram_idx = (CURR_LEVEL - BASE::MGRAM_IDX_OFFSET);

                    LOG_DEBUG2 << "Running post actions on " << CURR_LEVEL << "-grams, m-gram array index: " << mgram_idx << END_LOG;

                    //Sort the entries per context with respect to the word index
                    //the order could be arbitrary if a non-basic word index is used!
                    const size_t num_prev_ctx = (CURR_LEVEL == M_GRAM_LEVEL_2) ? m_one_gram_arr_size : m_M_N_gram_num_ctx_ids[mgram_idx - 1];
                    LOG_DEBUG2 << "Number of previous contexts: " << num_prev_ctx << END_LOG;
                    for (size_t ctx_id = 0; ctx_id < num_prev_ctx; ++ctx_id) {
                        const TSubArrReference & info = m_M_gram_ctx_2_data[mgram_idx][ctx_id];
                        if (info.beginIdx != BASE::UNDEFINED_ARR_IDX) {
                            LOG_DEBUG3 << "Sorting for context id: " << ctx_id << ", info.beginIdx: "
                                    << info.beginIdx << ", info.endIdx: " << info.endIdx << END_LOG;
                            my_sort<TWordIdPBEntry>(&m_M_gram_data[mgram_idx][info.beginIdx], (info.endIdx - info.beginIdx) + 1);
                        }
                    }
                }

                inline void post_n_grams() {
                    LOG_DEBUG2 << "Sorting the N-gram's data: ptr: " << m_N_gram_data
                            << ", size: " << m_M_N_gram_num_ctx_ids[BASE::N_GRAM_IDX_IN_M_N_ARR] << END_LOG;

                    //Order the N-gram array as it is unordered and we will binary search it later!
                    //Note: We dot not use Q-sort as it needs quite a lot of extra memory!
                    //Also, I did not yet see any performance advantages compared to sort!
                    //Actually the qsort provided here was 50% slower on a 20 Gb language
                    //model when compared to the str::sort!
                    my_sort<TCtxIdProbEntry>(m_N_gram_data, m_M_N_gram_num_ctx_ids[BASE::N_GRAM_IDX_IN_M_N_ARR]);
                };

            private:

                //Stores the 1-gram data
                T_M_Gram_Payload * m_1_gram_data;

                //Stores the M-gram context to data mappings for: 1 < M < N
                //This is a two dimensional array
                TSubArrReference * m_M_gram_ctx_2_data[BASE::NUM_M_GRAM_LEVELS];
                //Stores the M-gram data for the M levels: 1 < M < N
                //This is a two dimensional array
                TWordIdPBEntry * m_M_gram_data[BASE::NUM_M_GRAM_LEVELS];

                //Stores the N-gram data
                TCtxIdProbEntry * m_N_gram_data;

                //Stores the size of the One-gram
                TShortId m_one_gram_arr_size;
                //Stores the maximum number of context id  per M-gram level: 1 < M <= N
                TShortId m_M_N_gram_num_ctx_ids[BASE::NUM_M_N_GRAM_LEVELS];
                //Stores the context id counters per M-gram level: 1 < M <= N
                TShortId m_M_N_gram_next_ctx_id[BASE::NUM_M_N_GRAM_LEVELS];
            };

            typedef C2WArrayTrie<M_GRAM_LEVEL_MAX, BasicWordIndex > TC2WArrayTrieBasic;
            typedef C2WArrayTrie<M_GRAM_LEVEL_MAX, CountingWordIndex > TC2WArrayTrieCount;
            typedef C2WArrayTrie<M_GRAM_LEVEL_MAX, TOptBasicWordIndex > TC2WArrayTrieOptBasic;
            typedef C2WArrayTrie<M_GRAM_LEVEL_MAX, TOptCountWordIndex > TC2WArrayTrieOptCount;

        }
    }
}


#endif	/* CONTEXTTOWORDHYBRIDMEMORYTRIE_HPP */

