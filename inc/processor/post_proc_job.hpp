/* 
 * File:   post_proc_job.hpp
 * Author: zapreevis
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
 * Created on July 26, 2016, 4:19 PM
 */

#ifndef POST_PROC_JOB_HPP
#define POST_PROC_JOB_HPP

#include "common/utils/exceptions.hpp"
#include "common/utils/logging/logger.hpp"
#include "common/utils/threads/threads.hpp"

#include "processor/processor_job.hpp"
#include "processor/messaging/proc_req_in.hpp"

using namespace std;
using namespace uva::utils::exceptions;
using namespace uva::utils::logging;
using namespace uva::utils::threads;
using namespace uva::smt::bpbd::processor::messaging;

namespace uva {
    namespace smt {
        namespace bpbd {
            namespace processor {

                /**
                 * This is the post-processor job:
                 * Responsibilities:
                 *    - Stores the post-processor job specific data
                 *    - Execute the post-processor job
                 *    - Gather the results of the job
                 *    - Send the results back to the client 
                 */
                class post_proc_job : public processor_job {
                public:

                    /**
                     * The basic constructor
                     * @param config the language configuration, might be undefined.
                     * @param session_id the id of the session from which the translation request is received
                     * @param req the pointer to the post-processor request, not NULL
                     */
                    post_proc_job(const language_config & config,
                            const session_id_type session_id, proc_req_in * req)
                    : processor_job(config, session_id, req) {
                    }

                    /**
                     * The basic destructor
                     */
                    virtual ~post_proc_job() {
                    }

                    /**
                     * @see processor_job
                     */
                    virtual void execute() override {
                        //ToDo: Implement
                        THROW_NOT_IMPLEMENTED();
                    }

                    /**
                     * @see processor_job
                     */
                    virtual void synch_job_finished() override {
                        //ToDo: Implement
                        THROW_NOT_IMPLEMENTED();
                    }

                    /**
                     * @see processor_job
                     */
                    virtual void cancel() override {
                        //ToDo: Implement
                        THROW_NOT_IMPLEMENTED();
                    }
                };
            }
        }
    }
}

#endif /* PRE_PROC_JOB_HPP */

