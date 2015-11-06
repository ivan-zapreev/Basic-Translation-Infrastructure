/* 
 * File:   MGramSingleQuery.hpp
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
 * Created on November 5, 2015, 3:59 PM
 */

#ifndef MGRAMSINGLEQUERY_HPP
#define	MGRAMSINGLEQUERY_HPP

#include <string>       //std::string

#include "Globals.hpp"
#include "Exceptions.hpp"
#include "Logger.hpp"

#include "MGramQuery.hpp"
#include "QueryMGram.hpp"
#include "TextPieceReader.hpp"
#include "MGramQuery.hpp"

#include "AWordIndex.hpp"
#include "BasicWordIndex.hpp"
#include "CountingWordIndex.hpp"
#include "OptimizingWordIndex.hpp"

#include "C2DHashMapTrie.hpp"
#include "W2CHybridMemoryTrie.hpp"
#include "C2WOrderedArrayTrie.hpp"
#include "W2COrderedArrayTrie.hpp"
#include "C2DMapArrayTrie.hpp"
#include "LayeredTrieDriver.hpp"
#include "G2DHashMapTrie.hpp"
#include "GenericTrieDriver.hpp"

using namespace std;
using namespace uva::smt::logging;
using namespace uva::smt::file;
using namespace uva::smt::tries;
using namespace uva::smt::tries::dictionary;
using namespace uva::smt::tries::m_grams;
using namespace uva::utils::math::bits;
using namespace uva::smt::exceptions;

namespace uva {
    namespace smt {
        namespace tries {

            /**
             * Stores the query and its internal for the sake of re-usability and
             * independency from the Tries and executor. Allows to compute 
             *      log_10(Prob(w_{5}|w_{1}w_{2}w_{3}w_{4}))
             * Note that, here 5 is taken just as an example.
             */
            template<typename TrieType>
            class T_M_Gram_Single_Query : public T_M_Gram_Query<TrieType> {
            public:
                //The word index type
                typedef typename TrieType::WordIndexType WordIndexType;
                //Define the base class type
                typedef T_M_Gram_Query<TrieType> BASE;

                //Define the maximum level constant
                static constexpr TModelLevel MAX_LEVEL = TrieType::MAX_LEVEL;

                /**
                 * The basic constructor for the structure
                 * @param trie the reference to the trie object
                 */
                T_M_Gram_Single_Query(TrieType & trie) : T_M_Gram_Query<TrieType>(trie) {
                }

                /**
                 * Allows to log the query results after its execution.
                 * Different logging is done based on enabled logging level
                 * and the class template parameters.
                 */
                inline void log_results() const {
                    //Print the query results
                    const string gram_str = BASE::m_gram.get_mgram_prob_str(BASE::m_gram.get_m_gram_level());
                    
                    LOG_RESULT << "  log_" << LOG_PROB_WEIGHT_BASE << "( Prob( " << gram_str
                            << " ) ) = " << SSTR(m_prob) << END_LOG;
                    LOG_INFO << "  Prob( " << gram_str << " ) = "
                            << SSTR(pow(LOG_PROB_WEIGHT_BASE, m_prob)) << END_LOG;
                    
                    LOG_RESULT << "-------------------------------------------" << END_LOG;
                }

                /**
                 * Allows to execute m-gram the query
                 */
                void execute() {
                    LOG_DEBUG << "Starting to execute:" << (string) BASE::m_gram << END_LOG;

                    //Prepare the m-gram for querying
                    BASE::m_gram.template prepare_for_querying<true>();

                    //Initialize the begin and end index variables
                    TModelLevel begin_word_idx = BASE::m_gram.get_begin_word_idx();
                    TModelLevel end_word_idx = BASE::m_gram.get_end_word_idx();

                    //Clean the sub-m-gram probability array
                    m_prob = ZERO_PROB_WEIGHT;

                    LOG_DEBUG << "-----> Considering cumulative sub-m-gram [" << SSTR(begin_word_idx)
                            << ", " << SSTR(end_word_idx) << "]" << END_LOG;

                    THROW_NOT_IMPLEMENTED();
                    //Check if the en word is unknown
                    /*if (begin_word_idx > end_word_idx) {
                        //-------------------------------------------------------------------------------------
                        //ToDo: We actually need to get all the back-offs if present and
                        //      not just set the value to <unk> probability! This means we
                        //      need all the m-gram word ids and we also need to be able
                        //      to check if the given m-gram contains <unk> words.
                        //-------------------------------------------------------------------------------------
                        //The last word is unknown so no need to do any back-off,  
                        do_last_word_unknown(begin_word_idx, end_word_idx);
                    } else {
                        //The last word is known, try to retrieve the payload
                        if (BASE::m_add_prob_get_back_off[begin_word_idx][end_word_idx](BASE::m_trie, BASE::m_gram, BASE::m_payload, m_prob[end_word_idx])) {
                            //If the sub-m-gram payload is not defined then back-off
                            do_back_off_undefined(begin_word_idx, end_word_idx);
                        }
                    }*/
                }

            private:
                //Stores the probability result for the m-grams
                TLogProbBackOff m_prob;
            };

            //Make sure that there will be templates instantiated, at least for the given parameter values
#define INSTANTIATE_TYPEDEF_M_GRAM_SINGLE_QUERY_LEVEL_WORD_IDX(M_GRAM_LEVEL, WORD_INDEX_TYPE); \
            template class T_M_Gram_Single_Query<GenericTrieDriver<LayeredTrieDriver<C2DHybridTrie<M_GRAM_LEVEL, WORD_INDEX_TYPE>>>>; \
            template class T_M_Gram_Single_Query<GenericTrieDriver<LayeredTrieDriver<C2DMapTrie<M_GRAM_LEVEL, WORD_INDEX_TYPE>>>>; \
            template class T_M_Gram_Single_Query<GenericTrieDriver<LayeredTrieDriver<C2WArrayTrie<M_GRAM_LEVEL, WORD_INDEX_TYPE>>>>; \
            template class T_M_Gram_Single_Query<GenericTrieDriver<LayeredTrieDriver<W2CArrayTrie<M_GRAM_LEVEL, WORD_INDEX_TYPE>>>>; \
            template class T_M_Gram_Single_Query<GenericTrieDriver<LayeredTrieDriver<W2CHybridTrie<M_GRAM_LEVEL, WORD_INDEX_TYPE>>>>; \
            template class T_M_Gram_Single_Query<GenericTrieDriver<G2DMapTrie<M_GRAM_LEVEL, WORD_INDEX_TYPE>>>;

#define INSTANTIATE_TYPEDEF_M_GRAM_SINGLE_QUERY_LEVEL(M_GRAM_LEVEL); \
            INSTANTIATE_TYPEDEF_M_GRAM_SINGLE_QUERY_LEVEL_WORD_IDX(M_GRAM_LEVEL, BasicWordIndex); \
            INSTANTIATE_TYPEDEF_M_GRAM_SINGLE_QUERY_LEVEL_WORD_IDX(M_GRAM_LEVEL, CountingWordIndex); \
            INSTANTIATE_TYPEDEF_M_GRAM_SINGLE_QUERY_LEVEL_WORD_IDX(M_GRAM_LEVEL, TOptBasicWordIndex); \
            INSTANTIATE_TYPEDEF_M_GRAM_SINGLE_QUERY_LEVEL_WORD_IDX(M_GRAM_LEVEL, TOptCountWordIndex);

            INSTANTIATE_TYPEDEF_M_GRAM_SINGLE_QUERY_LEVEL(M_GRAM_LEVEL_MAX);
        }
    }
}

#endif	/* MGRAMSINGLEQUERY_HPP */

