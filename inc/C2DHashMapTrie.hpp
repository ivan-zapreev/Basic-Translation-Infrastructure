/* 
 * File:   C2DHashMapTrie.hpp
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
 * Created on August 14, 2015, 1:53 PM
 */

#ifndef C2DHASHMAPTRIE_HPP
#define	C2DHASHMAPTRIE_HPP

/**
 * We actually have several choices:
 * 
 * Continue to use <ext/hash_map.h> and use -Wno-deprecated to stop the warning
 * 
 * Use <tr1/unordered_map> and std::tr1::unordered_map
 * 
 * Use <unordered_map> and std::unordered_map and -std=c++0x
 * 
 * We will need to test which one runs better, it is an unordered_map for now.
 * http://www.cplusplus.com/reference/unordered_map/unordered_map/
 */
#include <utility>        // std::pair, std::make_pair
#include <unordered_map>  // std::unordered_map

#include "Globals.hpp"
#include "Logger.hpp"
#include "ALayeredTrie.hpp"
#include "GreedyMemoryAllocator.hpp"
#include "HashingUtils.hpp"
#include "TextPieceReader.hpp"

using namespace std;
using namespace uva::smt::hashing;
using namespace uva::smt::logging;
using namespace uva::smt::tries::alloc;

namespace uva {
    namespace smt {
        namespace tries {

            /**
             * This is a HashMpa based ATrie interface implementation class.
             * Note 1: This implementation uses the unsigned long for the hashes it is not optimal
             * Note 2: the unordered_map might be not as efficient as a hash_map with respect to memory usage but it is supposed to be faster
             * 
             * This implementation is chosen because it resembles the ordered array implementation from:
             *      "Faster and Smaller N -Gram Language Models"
             *      Adam Pauls Dan Klein
             *      Computer Science Division
             *      University of California, Berkeley
             * 
             * and unordered_maps showed good performance in:
             *      "Efficient in-memory data structures for n-grams indexing"
             *       Daniel Robenek, Jan Platoˇs, and V ́aclav Sn ́aˇsel
             *       Department of Computer Science, FEI, VSB – Technical University of Ostrava
             *       17. listopadu 15, 708 33, Ostrava-Poruba, Czech Republic
             *       {daniel.robenek.st, jan.platos, vaclav.snasel}@vsb.cz
             * 
             */
            template<TModelLevel N>
            class C2DHashMapTrie : public ALayeredTrie<N> {
            public:

                /**
                 * The basic class constructor, accepts memory factors that are the
                 * coefficients used when pre-allocating memory for unordered maps.
                 * 
                 * If a factor is equal to 0.0 then no memory is pre-allocated.
                 * If the factor is equal to 1.0 then there is only as much preallocated
                 * as needed to store the gram entries. The latter is typically not enough
                 * as unordered_map needs more memory for internal administration.
                 * If there is not enough memory pre-allocated then additional allocations
                 * will take place but it does not alway lead to more efficient memory
                 * usage. The observed behavior is that it is better to pre-allocate
                 * a bit more memory beforehand, than needed. This leads to less
                 * memory consumption. Depending on the type of unordered_map
                 * key/value pair types the advised factor values are from 2.0 to 2.6.
                 * Because it can not be optimally determined beforehand, these are made
                 * constructor parameters so that they can be configured by the used.
                 * This breaks encapsulation a bit, exposing the internals, but
                 * there is no other better way, for fine tuning the memory usage.
                 * 
                 * @param _pWordIndex the word index to be used
                 * @param _mGramMemFactor The M-Gram memory factor needed for
                 * the greedy allocator for the unordered_map
                 * @param _nGramMemFactor The N-Gram memory factor needed for
                 * the greedy allocator for the unordered_map
                 */
                explicit C2DHashMapTrie(AWordIndex * const _pWordIndex,
                        const float _mGramMemFactor = __CtxMultiHashMapTrie::UM_M_GRAM_MEMORY_FACTOR,
                        const float _nGramMemFactor = __CtxMultiHashMapTrie::UM_N_GRAM_MEMORY_FACTOR);

                /**
                 * This method can be used to provide the N-gram count information
                 * That should allow for pre-allocation of the memory
                 * For more details @see ATrie
                 */
                virtual void pre_allocate(const size_t counts[N]);

                /**
                 * The basic destructor
                 */
                virtual ~C2DHashMapTrie();

            protected:

                /**
                 * Allows to retrieve the data storage structure for the One gram with the given Id.
                 * If the storage structure does not exist, return a new one.
                 * For more details @see ATrie
                 */
                virtual TProbBackOffEntry & make_1_GramDataRef(const TShortId wordId) {
                    if (DO_SANITY_CHECKS) {
                        //Add hash key statistics
                        if (Logger::isRelevantLevel(DebugLevelsEnum::INFO3)) {
                            hashSizes[0].first = min<TLongId>(wordId, hashSizes[0].first);
                            hashSizes[0].second = max<TLongId>(wordId, hashSizes[0].second);
                        }
                    }

                    //Get the word probability and back-off data reference
                    return m_1_gram_data[wordId];
                };

                /**
                 * Allows to retrieve the data storage structure for the One gram with the given Id.
                 * If the storage structure does not exist, throws an exception.
                 * For more details @see ATrie
                 */
                virtual bool get_1_GramDataRef(const TShortId wordId, const TProbBackOffEntry ** ppData) {
                    //The data is always present.
                    *ppData = &m_1_gram_data[wordId];
                    return true;
                };

                /**
                 * Allows to retrieve the data storage structure for the M gram
                 * with the given M-gram level Id. M-gram context and last word Id.
                 * If the storage structure does not exist, return a new one.
                 * For more details @see ATrie
                 */
                virtual TProbBackOffEntry & make_M_GramDataRef(const TModelLevel level, const TShortId wordId, TLongId ctxId) {
                    //Store the N-tires from length 2 on and indexing starts
                    //with 0, therefore "level-2". Get/Create the mapping for this
                    //word in the Trie level of the N-gram

                    //Note: there is no need to check on the result of the function
                    //as in this Trie the context can always be computed!
                    (void) getContextId(wordId, ctxId);

                    //Add hash key statistics
                    if (DO_SANITY_CHECKS && Logger::isRelevantLevel(DebugLevelsEnum::INFO3)) {
                        hashSizes[level - 1].first = min<TLongId>(ctxId, hashSizes[level - 1].first);
                        hashSizes[level - 1].second = max<TLongId>(ctxId, hashSizes[level - 1].second);
                    }

                    return pMGramMap[level - ALayeredTrie<N>::MGRAM_IDX_OFFSET]->operator[](ctxId);
                };

                /**
                 * Allows to retrieve the data storage structure for the M gram
                 * with the given M-gram level Id. M-gram context and last word Id.
                 * If the storage structure does not exist, throws an exception.
                 * For more details @see ATrie
                 */
                virtual bool get_M_GramDataRef(const TModelLevel level, const TShortId wordId,
                        TLongId ctxId, const TProbBackOffEntry **ppData) {
                    //Get the next context id
                    if (getContextId(wordId, ctxId)) {
                        //Search for the map for that context id
                        const TModelLevel idx = (level - ALayeredTrie<N>::MGRAM_IDX_OFFSET);
                        TMGramsMap::const_iterator result = pMGramMap[idx]->find(ctxId);
                        if (result == pMGramMap[idx]->end()) {
                            //There is no data found under this context
                            return false;
                        } else {
                            //There is data found under this context
                            *ppData = &result->second;
                            return true;
                        }
                    } else {
                        //The context id could not be found
                        return false;
                    }
                };

                /**
                 * Allows to retrieve the data storage structure for the N gram.
                 * Given the N-gram context and last word Id.
                 * If the storage structure does not exist, return a new one.
                 * For more details @see ATrie
                 */
                virtual TLogProbBackOff & make_N_GramDataRef(const TShortId wordId, TLongId ctxId) {
                    //Data stores the N-tires from length 2 on, therefore "idx-1"
                    //Get/Create the mapping for this word in the Trie level of the N-gram

                    //Note: there is no need to check on the result of the function
                    //as in this Trie the context can always be computed!
                    (void) getContextId(wordId, ctxId);

                    //Add hash key statistics
                    if (DO_SANITY_CHECKS && Logger::isRelevantLevel(DebugLevelsEnum::INFO3)) {
                        hashSizes[N - 1].first = min<TLongId>(ctxId, hashSizes[N - 1].first);
                        hashSizes[N - 1].second = max<TLongId>(ctxId, hashSizes[N - 1].second);
                    }

                    return pNGramMap->operator[](ctxId);
                };

                /**
                 * Allows to retrieve the probability value for the N gram defined by the end wordId and ctxId.
                 * For more details @see ATrie
                 */
                virtual bool get_N_GramProb(const TShortId wordId, TLongId ctxId,
                        TLogProbBackOff & prob) {
                    //Get the next context id
                    if (getContextId(wordId, ctxId)) {
                        //Search for the map for that context id
                        TNGramsMap::const_iterator result = pNGramMap->find(ctxId);
                        if (result == pNGramMap->end()) {
                            //There is no data found under this context
                            return false;
                        } else {
                            //There is data found under this context
                            prob = result->second;
                            return true;
                        }
                    } else {
                        //The context id could not be found
                        return false;
                    }
                };

            private:
                //The M-Gram memory factor needed for the greedy allocator for the unordered_map
                const float mGramMemFactor;
                //The N-Gram memory factor needed for the greedy allocator for the unordered_map
                const float nGramMemFactor;

                //Stores the 1-gram data
                TProbBackOffEntry * m_1_gram_data;

                //The type of key,value pairs to be stored in the M Grams map
                typedef pair< const TLongId, TProbBackOffEntry> TMGramEntry;
                //The typedef for the M Grams map allocator
                typedef GreedyMemoryAllocator< TMGramEntry > TMGramAllocator;
                //The N Grams map type
                typedef unordered_map<TLongId, TProbBackOffEntry, std::hash<TLongId>, std::equal_to<TLongId>, TMGramAllocator > TMGramsMap;
                //The actual data storage for the M Grams for 1 < M < N
                TMGramAllocator * pMGramAlloc[N - ALayeredTrie<N>::MGRAM_IDX_OFFSET];
                //The array of maps map storing M-grams for 1 < M < N
                TMGramsMap * pMGramMap[N - ALayeredTrie<N>::MGRAM_IDX_OFFSET];

                //The type of key,value pairs to be stored in the N Grams map
                typedef pair< const TLongId, TLogProbBackOff> TNGramEntry;
                //The typedef for the N Grams map allocator
                typedef GreedyMemoryAllocator< TNGramEntry > TNGramAllocator;
                //The N Grams map type
                typedef unordered_map<TLongId, TLogProbBackOff, std::hash<TLongId>, std::equal_to<TLongId>, TNGramAllocator > TNGramsMap;
                //The actual data storage for the N Grams
                TNGramAllocator * pNGramAlloc;
                //The map storing the N-Grams, they do not have back-off values
                TNGramsMap * pNGramMap;

                //The structure for storing the hash key values statistics
                pair<TLongId, TLongId> hashSizes[N];

                /**
                 * The copy constructor, is made private as we do not intend to copy this class objects
                 * @param orig the object to copy from
                 */
                C2DHashMapTrie(const C2DHashMapTrie & orig)
                : ALayeredTrie<N>(NULL, NULL), mGramMemFactor(0.0), nGramMemFactor(0.0), m_1_gram_data(NULL) {
                    throw Exception("ContextMultiHashMapTrie copy constructor must not be used, unless implemented!");
                };

                /**
                 * This method must used to provide the N-gram count information
                 * That should allow for pre-allocation of the memory
                 * @param counts the counts for the number of elements of each gram level
                 */
                void preAllocateOGrams(const size_t counts[N]);

                /**
                 * This method must used to provide the N-gram count information
                 * That should allow for pre-allocation of the memory
                 * @param counts the counts for the number of elements of each gram level
                 */
                void preAllocateMGrams(const size_t counts[N]);

                /**
                 * This method must used to provide the N-gram count information
                 * That should allow for pre-allocation of the memory
                 * @param counts the counts for the number of elements of each gram level
                 */
                void preAllocateNGrams(const size_t counts[N]);

                /**
                 * Computes the N-Gram context using the previous context and the current word id
                 * 
                 * WARNING: Must only be called for the M-gram level M > 1!
                 * 
                 * @param wordId the current word id
                 * @param ctxId [in] - the previous context id, [out] - the next context id
                 * @param level the M-gram level we are working with M, default UNDEF_NGRAM_LEVEL
                 * @return the resulting context
                 * @throw nothing
                 */
                static inline bool getContextId(const TShortId wordId, TLongId & ctxId, const TModelLevel level = UNDEF_NGRAM_LEVEL) {
                    //Use the Szudzik algorithm as it outperforms Cantor
                    ctxId = szudzik(wordId, ctxId);
                    //The context can always be computed
                    return true;
                }
            };
        }
    }
}
#endif	/* CONTEXTMULTIHASHMAPTRIE_HPP */
