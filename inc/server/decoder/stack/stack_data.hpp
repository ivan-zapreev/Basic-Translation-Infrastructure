/* 
 * File:   stack_data.hpp
 * Author: zapreevis
 *
 * Created on February 29, 2016, 3:52 PM
 */

#ifndef STACK_DATA_HPP
#define	STACK_DATA_HPP

#include <functional>

#include "common/utils/threads.hpp"

#include "server/decoder/sentence/sentence_data_map.hpp"
#include "server/rm/proxy/rm_query_proxy.hpp"
#include "server/lm/proxy/lm_fast_query_proxy.hpp"

using namespace std;

using namespace uva::utils::threads;

using namespace uva::smt::bpbd::server::rm::proxy;
using namespace uva::smt::bpbd::server::lm::proxy;

using namespace uva::smt::bpbd::server::decoder::sentence;

namespace uva {
    namespace smt {
        namespace bpbd {
            namespace server {
                namespace decoder {
                    namespace stack {

                        //Forward declaration of the class
                        template<size_t NUM_WORDS_PER_SENTENCE, size_t MAX_M_GRAM_QUERY_LENGTH>
                        class stack_state_templ;

                        //Typedef the multi state and instantiate it with the maximum number of words per sentence and
                        //the maximum LM query length as this is the max LM level - 1 plus the max target phrase length
                        typedef stack_state_templ<MAX_WORDS_PER_SENTENCE, LM_MAX_QUERY_LEN> stack_state;

                        //Define the multi state pointer
                        typedef stack_state * stack_state_ptr;

                        //The typedef for a function that adds a new state to the multi-stack
                        typedef function<void(stack_state_ptr) > add_new_state_function;

                        /**
                         * This structure stores the shared stack-state data.
                         * This data is valid within one sentence translation
                         * and is needed by multiple states and etc
                         */
                        struct stack_data {

                            /**
                             * The basic constructor to initialize the stored references
                             * @param params the decoder parameters
                             * @param is_stop the stopping flag
                             * @param sent_data the sentence data
                             * @param rm_query the reordering model query
                             * @param lm_query the language model query to be used
                             * @param add_state the function needed to add new states
                             */
                            stack_data(const de_parameters & params, acr_bool_flag is_stop,
                                    const sentence_data_map & sent_data, const rm_query_proxy & rm_query,
                                    lm_fast_query_proxy & lm_query, const add_new_state_function & add_state)
                            : m_params(params), m_is_stop(is_stop), m_sent_data(sent_data),
                            m_rm_query(rm_query), m_lm_query(lm_query), m_add_state(add_state) {
                            }

                            //The decoder parameters
                            const de_parameters & m_params;
                            
                            //The stopping flag
                            acr_bool_flag m_is_stop;
                            
                            //The sentence data
                            const sentence_data_map & m_sent_data;
                            
                            //The reordering model query
                            const rm_query_proxy & m_rm_query;
                            
                            //The language model query to be used
                            lm_fast_query_proxy & m_lm_query;
                            
                            //The function needed to add new states
                            const add_new_state_function m_add_state;
                        };
                    }
                }
            }
        }
    }
}

#endif	/* STACK_DATA_HPP */

