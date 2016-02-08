/* 
 * File:   tm_proxy_local.hpp
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
 * Created on February 8, 2016, 9:59 AM
 */

#ifndef TM_PROXY_LOCAL_HPP
#define TM_PROXY_LOCAL_HPP

#include "common/utils/logging/logger.hpp"
#include "common/utils/exceptions.hpp"
#include "common/utils/monitore/statistics_monitore.hpp"
#include "common/utils/file/cstyle_file_reader.hpp"

#include "server/tm/proxy/tm_query_proxy.hpp"
#include "server/tm/proxy/tm_query_proxy_local.hpp"

#include "server/tm/builders/tm_basic_builder.hpp"

using namespace uva::utils::monitore;
using namespace uva::utils::exceptions;
using namespace uva::utils::logging;
using namespace uva::utils::file;
using namespace uva::smt::translation::server::tm;
using namespace uva::smt::translation::server::tm::proxy;
using namespace uva::smt::translation::server::tm::builders;

namespace uva {
    namespace smt {
        namespace translation {
            namespace server {
                namespace tm {
                    namespace proxy {

                        /**
                         * This is the translation model proxy interface class it allows to
                         * interact with any sort of local and remote models in a uniform way.
                         */
                        template<typename model_type>
                        class tm_proxy_local : public tm_proxy {
                        public:
                            //Define the builder type 
                            typedef tm_basic_builder<model_type, CStyleFileReader> builder_type;

                            /**
                             * The basic proxy constructor, currently does nothing except for default initialization
                             */
                            tm_proxy_local() {
                            }

                            /**
                             * Allows to connect to the model object based on the given parameters
                             * @param the parameters defining the model to connect to
                             */
                            virtual void connect(const tm_parameters & params) {
                                //The whole purpose of this method connect here is
                                //just to load the translation model into the memory.
                                load_model_data<builder_type, CStyleFileReader>("Translation Model", params.m_conn_string);
                            }

                            /**
                             * Allows to disconnect from the trie
                             */
                            virtual void disconnect() {
                                //Nothing the be done here yet, the model is allocated on the stack
                            }

                            /**
                             * The basic virtual destructor
                             */
                            virtual ~tm_proxy_local() {
                                //Disconnect, just in case it has not been done before
                                disconnect();
                            };

                            /**
                             * This method allows to get a query executor for the given trie
                             * @return the trie query proxy object
                             */
                            virtual tm_query_proxy * get_query_proxy() {
                                //Return an instance of the query_proxy_local class
                                return new tm_query_proxy_local<model_type>(m_model);
                            }

                        protected:

                            /**
                             * Allows to load the model into the instance of the selected container class
                             * @param the name of the model being loaded
                             * @params model_file_name the model file name
                             */
                            template<typename builder_type, typename file_reader_type>
                            void load_model_data(char const *model_name, const string & model_file_name) {
                                //Declare time variables for CPU times in seconds
                                double start_time, end_time;
                                //Declare the statistics monitor and its data
                                TMemotyUsage mem_stat_start = {}, mem_stat_end = {};

                                //ToDo: Add the possibility to choose between the file readers from the command line!
                                LOG_DEBUG << "Getting the memory statistics before opening the " << model_name << " file ..." << END_LOG;
                                StatisticsMonitor::getMemoryStatistics(mem_stat_start);

                                //Attempt to open the model file
                                file_reader_type model_file(model_file_name.c_str());
                                model_file.log_reader_type_info();
                                LOG_DEBUG << "Getting the memory statistics after opening the " << model_name << " file ..." << END_LOG;
                                StatisticsMonitor::getMemoryStatistics(mem_stat_end);

                                //Log the usage information
                                m_model.log_model_type_info();

                                LOG_USAGE << "Start creating and loading the Trie ..." << END_LOG;
                                LOG_DEBUG << "Getting the memory statistics before loading the " << model_name << " ..." << END_LOG;
                                StatisticsMonitor::getMemoryStatistics(mem_stat_start);
                                LOG_DEBUG << "Getting the time statistics before creating the " << model_name << " ..." << END_LOG;
                                start_time = StatisticsMonitor::getCPUTime();

                                //Assert that the model file is opened
                                ASSERT_CONDITION_THROW(!model_file.is_open(), string("The ") + string(model_name) + string(" file: '")
                                        + model_file_name + string("' does not exist!"));

                                //Create the trie builder and give it the trie
                                builder_type builder(m_model, model_file);
                                //Load the model from the file
                                builder.build();

                                LOG_DEBUG << "Getting the time statistics after loading the " << model_name << " ..." << END_LOG;
                                end_time = StatisticsMonitor::getCPUTime();
                                LOG_USAGE << "Reading the " << model_name << " took " << (end_time - start_time) << " CPU seconds." << END_LOG;
                                LOG_DEBUG << "Getting the memory statistics after loading the " << model_name << " ..." << END_LOG;
                                StatisticsMonitor::getMemoryStatistics(mem_stat_end);
                                LOG_DEBUG << "Reporting on the memory consumption" << END_LOG;
                                const string action_name = string("Loading the ") + string(model_name);
                                report_memory_usage(action_name.c_str(), mem_stat_start, mem_stat_end, true);

                                LOG_DEBUG << "Getting the memory statistics before closing the " << model_name << " file ..." << END_LOG;
                                StatisticsMonitor::getMemoryStatistics(mem_stat_start);
                                LOG_DEBUG << "Closing the model file ..." << END_LOG;
                                model_file.close();
                                LOG_DEBUG << "Getting the memory statistics after closing the " << model_name << " file ..." << END_LOG;
                                StatisticsMonitor::getMemoryStatistics(mem_stat_end);
                            }

                        private:
                            //Stores the translation model instance
                            model_type m_model;
                        };
                    }
                }
            }
        }
    }
}

#endif /* TM_PROXY_LOCAL_HPP */
